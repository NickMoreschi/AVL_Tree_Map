#include <map> // helper container for thread copying

template <typename KEY, typename T>
Map<KEY, T>::Map(){
	_root = new Elem;
	_root->left = _root;
	_root->right = 0;
	_root->rightThread = false;
	_root->height = -1;
	_size = 0;
}

// copy constructor 
template <typename KEY, typename T>
Map<KEY, T>::Map(const Map<KEY, T> &v){
	if (v._root == v._root->left){
		_root = new Elem;
		_root->left = _root;
		_root->right = 0;
		_size = 0;
	} else {
		_root = new Elem;
		_root->left = _root;
		_root->right = 0;
		copyCode(_root->left, v._root->left);
		copyThread(_root, v._root);
		_size = v._size;
	}
}

template <typename KEY, typename T>
Map<KEY, T>::~Map() {
	if (_root->left != _root)  // not an empty tree
        destructCode(_root->left);
    delete _root;
}

template <typename KEY, typename T>
void Map<KEY, T>::destructCode(Elem *& p){
    if (p -> left) {
        destructCode(p -> left);
    }
    if (!p -> rightThread) {
        destructCode(p -> right);
    }
    delete p;
}

template <typename KEY, typename T>
typename Map<KEY, T>::Map& Map<KEY, T>::operator=(const Map &rhs) {
	if (&rhs == this) {
		return *this;
	}

	if(_root->left != _root) {
		destructCode(_root->left);
	}

	_size = rhs._size;

	if (_size >= 1) {
		copyCode(_root -> left, rhs._root -> left);
		copyThread(_root, rhs._root);
	}

	return *this;
}

// construct a key-element map to rethread the new tree
// the map contains all nodes key values and their corresonding elem node address 
// for furture lookup in setting up threads
template <typename KEY, typename T>
void Map<KEY, T>::addToMap(Elem* root, map<KEY, Elem*> &keyElemMap){
	if (root) {
		keyElemMap[root->key] = root; 
		addToMap(root->left, keyElemMap);
		if (!root->rightThread)
			addToMap(root->right, keyElemMap);
	}
}

// common copy code for thread copying
template <typename KEY, typename T>
void Map<KEY, T>::copyThread(Elem* &newRoot, Elem* origRoot){
	// construct the key-element map for new and orig tree
	map<KEY, Elem*> newKeyElemMap; 
	map<KEY, Elem*> origKeyElemMap;
	addToMap(newRoot->left, newKeyElemMap);
	addToMap(origRoot->left, origKeyElemMap);

	// start at the last element in the tree, which threads to root
	typename std::map<KEY, Elem*>::reverse_iterator it = origKeyElemMap.rbegin();
	newKeyElemMap[it->first] -> rightThread = true;
	newKeyElemMap[it->first] -> right = newRoot;
	
	// then thread the rest of the tree backwardly 
	it++;
	while(it != origKeyElemMap.rend()){
		if (it->second->rightThread){
			newKeyElemMap[it->first] -> rightThread = true;
			newKeyElemMap[it->first] -> right = newKeyElemMap[ origKeyElemMap[it->first]->right->key ];
		}
		it++;
	} 
}

// common copy code for deep copy a tree without copying threads
template <typename KEY, typename T>
void Map<KEY,T>::copyCode(Elem* &newRoot, Elem* origRoot){
	if (origRoot == 0)
		newRoot = 0;
	else{
		newRoot = new Elem;
		newRoot->key = origRoot->key;
		newRoot->data = origRoot->data;
		newRoot->rightThread = origRoot->rightThread; 
		copyCode(newRoot->left, origRoot->left);
		if (!origRoot->rightThread) 
			copyCode(newRoot->right, origRoot->right);
	}
}

template <typename KEY, typename T>
bool Map<KEY, T>::insert(KEY key, T data) {
	//if (find(key) == end()) {
		if (insert(_root->left, key, data, _root)){
        	_size++;
        	return true;
		}
	//}
    return false;
}

//insert helper method
template <typename KEY, typename T> //cur below could be root
bool Map<KEY, T>::insert(Elem *& cur, const KEY &key, const T &data, Elem *lastLeft) {
	if (find(key) != end()) {
		return false;
	}

    if (cur == _root || cur == 0) {
        cur = new Elem;
		cur->key = key;
        cur->data = data;
        cur->left = 0;
        cur->right = lastLeft;
        cur->rightThread = true;
		cur -> height = 0;
        return true;
    }
    
    if (key == cur->key){
        return false;
    }

    if (key < cur->key) {
        insert(cur->left, key, data, cur);
	}

    if (key > cur -> key && !cur->rightThread){
        insert(cur->right, key, data, lastLeft);
    }
	else if (key > cur -> key) {
        cur->rightThread = false;
        cur->right = new Elem;
        cur->right->data = data;
		cur->right->key = key;
        cur->right->left = 0;
        cur->right->right = lastLeft;
        cur->right->rightThread = true;
		cur->right -> height = 0;
    }

	if (!cur -> rightThread) {
		if (height(cur -> left) - height(cur -> right) == 2) {
        	if(key < cur -> left -> key) {
            	rotateRight(cur);
			} else {
        		doubleRotateRight(cur);
			}
    	}

		if (height(cur -> left) - height(cur -> right) == -2) {
        	if(key > cur -> right -> key) {
            	rotateLeft(cur);
			} else {
            	doubleRotateLeft(cur);
			}
    	}
	}
	else {
		if (height(cur -> left) + 1 == 2) {
			if (key < cur -> left -> key) {
				rotateRight(cur);
			}
			else {
				doubleRotateRight(cur);
			}
		}
		if (height(cur -> left) + 1 == -2) {
			if (key < cur -> right -> key) {
				rotateLeft(cur);
			}
			else {
				doubleRotateLeft(cur);
			}
		}
	}

	if (!cur -> rightThread) {
		cur -> height = max(height(cur -> left), height(cur -> right)) + 1;
	}
	else {
		cur -> height = height(cur -> left) + 1;
	}

	return true;
}

template <typename KEY, typename T>
typename Map<KEY, T>::Iterator Map<KEY, T>::find(KEY key) const {
	Elem *temp = _root -> left;
	while (temp) {
        if (key < temp -> key) {
            temp = temp -> left;
        }
        else if (key > temp -> key) {
			if (temp -> rightThread) {
				break;
			}
			else {
				temp = temp -> right;
			}
        }
        else if(key == temp -> key){
            Iterator it = Iterator(temp);
			return it;
        }
    }
	return end();
}

template <typename KEY, typename T>
typename Map<KEY, T>::Iterator Map<KEY, T>::begin() const {  // return the left most (smallest) tree node
	Elem *begin = _root;
	while (begin -> left != nullptr) {
		if (begin -> left != begin) {
			begin = begin -> left;
		}
		else {
			break;
		}
	}
	return Iterator(begin);
}

template <typename KEY, typename T>
typename Map<KEY, T>::Iterator Map<KEY, T>::end() const {  // return the dummy root node
	return Iterator(_root);
} 

template <typename KEY, typename T>
typename Map<KEY, T>::Iterator Map<KEY, T>::Iterator::operator++(int){
	Iterator it = *this;
	if (_cur -> rightThread) {
		if (_cur -> right) {
			_cur = _cur -> right;
		}
	}
	else {
		if (_cur -> right) {
			_cur = _cur -> right;
		}
		while (_cur -> left) {
			_cur = _cur -> left;
		}
	}
	return it;
}

template <typename KEY, typename T>
T& Map<KEY, T>::operator[](KEY key) {
	T var;
	Iterator it = find(key);
	if (it == end()) {
		insert(key, var);
		return find(key) -> data;
	}
	return it -> data;
}

template <typename KEY, typename T>
int Map<KEY, T>::height(Elem *node) {
    return node == 0 ? -1 : node->height;
}

// single right rotation
template <typename KEY, typename T>
void Map<KEY, T>::rotateRight(Elem *& node) {
	Elem *temp = node -> left;
	if (!temp -> rightThread) {
		node -> left = temp -> right;
	}
	else {
		node -> left = nullptr;
	}
    temp -> right = node;
	temp -> rightThread = false;
	if (node -> rightThread) {
		node -> height = max(height(node -> left), -1) + 1;
	}
	else {
		node -> height = max(height(node -> left), height(node -> right)) + 1;
	}
    node = temp;
}

// single left rotation
template <typename KEY, typename T>
void Map<KEY, T>::rotateLeft(Elem *& node){
    Elem *temp = node -> right;
	if (temp -> left) {
		node -> right = temp -> left;
	}
	else {
		node -> right = temp;
		node -> rightThread = true;
	}
    temp -> left = node;
	if (node -> rightThread) {
		node -> height = max(height(node -> left), -1) + 1;
	}
	else {
		node -> height = max(height(node -> left), height(node -> right)) + 1;
	}
    node = temp;
}

// double right rotation
template <typename KEY, typename T>
void Map<KEY, T>::doubleRotateRight(Elem *& node) {
    rotateLeft(node -> left);
    rotateRight(node);
}

// double left rotation
template <typename KEY, typename T>
void Map<KEY, T>::doubleRotateLeft(Elem *& node) {
    rotateRight(node -> right);
    rotateLeft(node);
}

template <typename KEY, typename T>
typename Map<KEY, T>::Elem& Map<KEY, T>::Iterator::operator*(){ 
	return *_cur;
}

template <typename KEY, typename T>
typename Map<KEY, T>::Elem* Map<KEY, T>::Iterator::operator->(){ 
	return _cur;
}

template <typename KEY, typename T>
bool Map<KEY, T>::Iterator::operator==(Iterator it){ 
	return _cur == it._cur;
}

template <typename KEY, typename T>
bool Map<KEY, T>::Iterator::operator!=(Iterator it){ 
	return _cur != it._cur;
}

template <typename KEY, typename T>
int Map<KEY, T>::size() const{
	return _size;
}

// output the structure of tree. The tree is output as "lying down"
// output each node's key, value and its tree height 
template <typename KEY, typename T>
void Map<KEY, T>::printTree(ostream& out, int level, Elem *p) const{
	int i;
	if (p) {
		if (p->right && !p->rightThread)
			printTree(out, level+1,p->right);
		for(i=0;i<level;i++) {
			out << "\t";
		}
		out << p->key << " " << p->data << "(" << p->height << ")" << '\n';
		printTree(out, level+1,p->left);
	}
}

// outputs information in tree in inorder traversal order
template <typename KEY, typename T>
ostream& Map<KEY, T>::dump(ostream& out) const{
	if ( _root == _root->left) {// tree empty
		return out;
	}
	printTree(out, 0, _root->left);   // print tree structure
	return out;
}


// outputs using overloaded << operator
template<typename KEY, typename T>
ostream& operator<< (ostream& out, const Map<KEY, T>& v){
	v.dump(out);
	return out;
}
