#ifndef SJTU_LRU_HPP
#define SJTU_LRU_HPP

#include "utility.hpp"
#include "exceptions.hpp"
#include "class-integer.hpp"
#include "class-matrix.hpp"
class Hash {
public:
	unsigned int operator () (Integer lhs) const {
		int val = lhs.val;
		return std::hash<int>()(val);
	}
};
class Equal {
public:
	bool operator () (const Integer &lhs, const Integer &rhs) const {
		return lhs.val == rhs.val;
	}
};

namespace sjtu {
template<class T> class double_list{
private:
	struct Node {
		T *data;
		Node *nxt,*pre;
		Node (){
			data = nullptr;
			nxt = pre = nullptr;
		}
		Node (T *dat, Node *nxt = nullptr, Node *pre = nullptr) : data(dat), nxt(nxt), pre(pre) {}
		~Node() {delete data;}
	};
	
public:
	
	// Acknowledgements: Thank Wang YuXuan for rasing the idea of use a tail_extra to express the end() 
	// tail->nxt always refers to tail_extra so that 
	Node tail_extra;
	Node *head, *tail;
	double_list(){
		head = tail = &tail_extra;
	}
	double_list(const double_list<T> &other){
		for(auto & u : other) {
			insert_tail(u);
		}
	}
	double_list & operator=(const double_list &other){
		this->destroy();
		for(auto & u : other) 
			insert_tail(u);
		return *this;
	}
	~double_list(){
		destroy();
	}

	class iterator{
	public:
    	Node * pos;

		iterator(){
			pos = nullptr;
		}

		iterator(Node *val) : pos(val){}
		iterator(const iterator &t){
			pos = t.pos;
		}
		~iterator(){}
        /**
		 * iter++
		 */
		iterator operator++(int) {
			iterator tmp = *this;
			pos = pos->nxt;
			return tmp;
		}
        /**
		 * ++iter
		 */
		iterator &operator++() {
			pos = pos->nxt;
			return *this;
		}
        /**
		 * iter--
		 */
		iterator operator--(int) {
			iterator tmp = *this;
			pos = pos->pre;
			return tmp;
		}
        /**
		 * --iter
		 */
		iterator &operator--() {
			pos = pos->pre;
			return *this;
		}
		/**
		 * if the iter didn't point to a value
		 * throw " invalid"
		*/
		T &operator*() const {
			if(!(pos->data)) throw "invalid";
			return *pos->data;
		}
        /**
         * other operation
        */
		T *operator->() const noexcept {
			return pos->data;
		}
		bool operator==(const iterator &rhs) const {
			return pos == rhs.pos;
		}
		bool operator!=(const iterator &rhs) const {
			return !(*this == rhs);
		}
	};
	/**
	 * return an iterator to the beginning
	 */
	iterator begin() const{
		return iterator(head);
	}
	/**
	 * return an iterator to the ending
	 * in fact, it returns the iterator point to nothing,
	 * just after the last element.
	 */
	iterator end() const{
		return iterator(tail);
	}
	/**
	 * if the iter didn't point to anything, do nothing,
	 * otherwise, delete the element pointed by the iter
	 * and return the iterator point at the same "index"
	 * e.g.
	 * 	if the origin iterator point at the 2nd element
	 * 	the returned iterator also point at the
	 *  2nd element of the list after the operation
	 *  or nothing if the list after the operation
	 *  don't contain 2nd elememt.
	*/
	iterator erase(iterator pos){
		if(pos == end() || pos == iterator()) return end();
		if(pos == begin()) {
			delete_head();
			return begin();
		}
 		Node *now = pos.pos;
		iterator res(now->nxt);
		(now->pre)->nxt = now->nxt;
		(now->nxt)->pre = now->pre;
		delete now;
		return res;
	}

	/**
	 * the following are operations of double list
	*/
	void insert_head(const T &val){
		Node *new_node = new Node(new T(val), head, nullptr);
		// new_node->nxt = head;
		head->pre = new_node;
		head = new_node;
	}
	void insert_tail(const T &val){
		Node *new_node = new Node(new T(val), tail, tail->pre);
		if(!(tail->pre)	) head = new_node;
		else tail->pre->nxt = new_node;
		tail->pre = new_node;
	}
	void delete_head(){
		if(head != tail) {
			Node *tmp = head;
			head = head->nxt;
			head->pre = nullptr;
			delete tmp;
		}
	}
	void delete_tail(){
		if(head != tail) {
			Node *tmp = tail->pre;
			if(!tmp->pre) head = tail;
			else tmp->pre->nxt = tail;
			tail->pre = tmp->pre;
			delete tmp;
		}
	}
	/**
	 * if didn't contain anything, return true, 
	 * otherwise false.
	 */
	bool empty(){
		return head == tail;
	}

	void destroy() {
		Node *pos = head;
		while(pos != tail) {
			pos = pos->nxt;
			delete pos->pre;
		}
		tail_extra = Node();
		head = tail = &tail_extra;
	}
};
static size_t capacity_list[25] = {
	0, 11, 23, 47, 101, 211, 421, 853, 1741, 3541, 7027, 14009, 28001, 56003, 112019, 224011, 448003, 896003, 1997619, 3999999, 8000001, 16000001, 32000003, 64000003, 128000001 	
};
template<
	class Key,
	class T,
	class Hash = std::hash<Key>, 
	class Equal = std::equal_to<Key>
> class hashmap{
public:
	using value_type = pair<const Key, T>;
	struct Node {
		value_type *data;
		Node *nxt;
		Node () {data = nullptr; nxt = nullptr;}
		Node (const value_type dat) : data(new value_type(dat)), nxt(nullptr) {}
		~Node () {delete data;}
	};
	Node ** hash_table;
	Node end_pos;
	size_t capacity, siz, expand_time;
	Hash hash_function;
	Equal equal_function;
public:
	hashmap(){
		expand_time = 1;
		capacity = capacity_list[expand_time]; 
		siz = 0;
		hash_table = new Node*[capacity]();
	}
	hashmap(const hashmap &other){
		expand_time = 1;
		capacity = capacity_list[expand_time]; 
		siz = 0;
		hash_table = new Node*[capacity]();
		for(size_t i = 0; i < other.capacity; i++) {
			Node *tmp = other.hash_table[i];
			while(tmp) {
				insert(*(tmp->data));
				tmp = tmp->nxt;
			}
		}
	}
	~hashmap(){
		clear();
		delete []hash_table;
	}
	hashmap & operator=(const hashmap &other){
		if(this != &other) {
			clear();
			for(size_t i = 0; i < other.capacity; i++) {
				Node *tmp = other.hash_table[i];
				while(tmp) {
					insert(*(tmp->data));
					tmp = tmp->nxt;
				}
			}
		}
		return *this;
	}

	class iterator{
	public:
    	Node * pos;

		iterator(){
			pos = nullptr;
		}
		iterator (Node *oth) {
			pos = oth;
		}
		iterator(const iterator &t){
			pos = t.pos;
		}
		~iterator(){}

        /**
		 * if point to nothing
		 * throw 
		*/
		value_type &operator*() const { 
			return *(pos->data);
		}

        /**
		 * other operation
		*/
		value_type *operator->() const noexcept {
			if(!pos->data) throw "invalid";
			return pos->data;
		}
		bool operator==(const iterator &rhs) const {
			return pos == rhs.pos;
    	}
		bool operator!=(const iterator &rhs) const {
			return !(pos == rhs.pos);
		}
	};

	void clear(){
		for(size_t i = 0; i < capacity; i++) {
			Node *tmp = hash_table [i];
			while(tmp) {
				Node * next = tmp->nxt;
				delete tmp;
				tmp = next;
			}
			hash_table[i] = nullptr;
		}
		siz = 0;
	}
	/**
	 * you need to expand the hashmap dynamically
	*/
	void expand(){
		expand_time++;
		size_t new_cap = capacity_list[expand_time];
		Node ** new_hashtable = new Node *[new_cap]();
		for(size_t i = 0; i < capacity; i++) {
			Node * tmp = hash_table[i];
			while(tmp) {
				Node * nxt = tmp -> nxt;
				size_t new_id = hash_function((*(tmp->data)).first) % new_cap;
				tmp->nxt = new_hashtable[new_id];
				new_hashtable[new_id]=tmp;
				tmp = nxt;
			}
		}
		delete [] hash_table;
		hash_table = new_hashtable;
		capacity = new_cap;
	}

    /**
     * the iterator point at nothing
    */
	iterator end() const{
		return iterator();
	}
	/**
	 * find, return a pointer point to the value
	 * not find, return the end (point to nothing)
	*/
	Node* find_pointer(const Key & key) const {
		size_t num = hash_function(key) % capacity;
		Node * tmp = hash_table[num];
		while(tmp) {
			if(equal_function((*(tmp->data)).first, key))
				return tmp;
			tmp = tmp->nxt;	
		}
		return nullptr;
	}
	iterator find(const Key & key) const {
		Node * node = find_pointer(key);
        if (node != nullptr)
            return iterator(node);
        return end();
	}
	/**
	 * already have a value_pair with the same key
	 * -> just update the value, return false
	 * not find a value_pair with the same key
	 * -> insert the value_pair, return true
	*/
	sjtu::pair<iterator,bool> insert(const value_type &value_pair){
		Node * node = find_pointer(value_pair.first);
        if (node != nullptr) {
            (*(node->data)).second = value_pair.second;
            return(sjtu::pair<iterator,bool>){iterator(node), false};
        }
        size_t index = hash_function(value_pair.first) % capacity;
        Node* newNode = new Node(value_pair);
        newNode->nxt = hash_table[index];
        hash_table[index] = newNode;
        ++siz;
        if (siz > capacity)
            expand();
        return(sjtu::pair<iterator,bool>){iterator(newNode), true};
	}
	/**
	 * the value_pair exists, remove and return true
	 * otherwise, return false
	*/
	bool remove(const Key &key){
		size_t num = hash_function(key) % capacity;
        Node *tmp = hash_table[num];
        Node *pre = nullptr;
        while(tmp) {
            if(equal_function((*(tmp->data)).first, key)) {
                if (pre) pre->nxt = tmp->nxt;
                else hash_table[num] = tmp->nxt;
                delete tmp; siz--;
                return true;
            }
            pre = tmp;
            tmp = tmp->nxt;
        }
        return false;
	}
};

template<
	class Key,
	class T,
	class Hash = std::hash<Key>, 
	class Equal = std::equal_to<Key>
> class linked_hashmap {
public:
	typedef pair<const Key, T> value_type;
	using list_val = double_list <value_type>;
	using list_iter = typename double_list <value_type> :: iterator;
	using map_iter = typename hashmap <Key, list_iter, Hash, Equal> :: iterator;
	using map_pair = pair<const Key, list_iter>;
	list_val lst;
	hashmap <Key, list_iter, Hash, Equal> mp;

public:
	class const_iterator;
	class iterator{
	public:
		list_iter pos;
		const list_val *id;
		iterator() {
			id = nullptr;
		}
		iterator(const list_val *id_, list_iter oth) {
			id = id_;
			pos = oth;
		}
		iterator(const iterator &other){
			pos = other.pos;
			id = other.id;
		}
		~iterator(){
		}
		/**
		 * iter++
		 */
		iterator operator++(int) {
			if(pos == list_iter() || pos == id->end()) throw "invalid";
			iterator tmp = *this;
			pos++;
			return tmp;
		}
        /**
		 * ++iter
		 */
		iterator &operator++() {
			if(pos == list_iter() || pos == id->end()) throw "invalid";
			pos++;
			return *this;
		}
        /**
		 * iter--
		 */
		iterator operator--(int) {
			if(pos == list_iter() || pos == id->begin()) throw "invalid";
			iterator tmp = *this;
			pos--;
			return tmp;
		}
        /**
		 * --iter
		 */
		iterator &operator--() {
			if(pos == list_iter() || pos == id->begin()) throw "invalid";
			pos--;
			return *this;
		}
		/**
		 * if the iter didn't point to a value
		 * throw "star invalid"
		*/
		value_type &operator*() {
			if(pos == list_iter() || pos == id->end()) throw "invalid";
			return *(pos.pos->data);
		}	
		value_type *operator->() const noexcept {
			return pos.pos->data;
		}

		/**
		 * operator to check whether two iterators are same (pointing to the same memory).
		 */
		bool operator==(const iterator &rhs) const {
			return pos == rhs.pos && id == rhs.id;
		}
		bool operator!=(const iterator &rhs) const {
			return !(*this == rhs);
		}
		bool operator==(const const_iterator &rhs) const {
			return pos == rhs.pos && id == rhs.id;
		}
		bool operator!=(const const_iterator &rhs) const {
			return !(*this == rhs);
		}
	};
 
	class const_iterator {
		public:
		list_iter pos;
		const list_val *id;
		const_iterator() {
			id = nullptr;
		}
		const_iterator(const list_val *id_, list_iter oth) {
			pos = oth;
			id = id_;
		}
		const_iterator(const iterator &other) {
			pos = other.pos;
			id = other.id;
		}

		/**
		 * iter++
		 */
		const_iterator operator++(int) {
			if(pos == list_iter() || pos == id->end()) throw "invalid";
			const_iterator tmp = *this;
			pos++;
			return tmp;
		}
		/**
		 * ++iter
		 */
		const_iterator &operator++() {
			if(pos == list_iter() || pos == id->end()) throw "invalid";
			pos++;
			return *this;
		}
		/**
		 * iter--
		 */
		const_iterator operator--(int) {
			if(pos == list_iter() || pos == id->begin()) throw "invalid";
			const_iterator tmp = *this;
			pos--;
			return tmp;
		}
		/**
		 * --iter
		 */
		const_iterator &operator--() {
			if(pos == list_iter() || pos == id->begin()) throw "invalid";
			pos--;
			return *this;
		}

		/**
		 * if the iter didn't point to a value
		 * throw 
		*/
		const value_type &operator*() const {
			if(pos == list_iter() || pos == id->end()) throw "invalid";
			return *(pos.pos->data);
		}
		const value_type *operator->() const noexcept {
			return pos.pos->data;
		}

		/**
		 * operator to check whether two iterators are same (pointing to the same memory).
		 */
		bool operator==(const iterator &rhs) const {
			return pos == rhs.pos && id == rhs.id;
		}
		bool operator!=(const iterator &rhs) const {
			return !(*this == rhs);
		}
		bool operator==(const const_iterator &rhs) const {
			return pos == rhs.pos && id == rhs.id;
		}
		bool operator!=(const const_iterator &rhs) const {
			return !(*this == rhs);
		}
	};
 
	linked_hashmap() {
	}
	linked_hashmap(const linked_hashmap &other){
		// std::cerr<<"??";
		lst = other.lst;
		mp = other.mp;
	}
	~linked_hashmap() {}
	linked_hashmap & operator=(const linked_hashmap &other) {
		if(this == &other) return *this;
		clear();
		lst = other.lst;
		mp = other.mp;
		return *this;
	}

 	/**
	 * return the value connected with the Key(O(1))
	 * if the key not found, throw 
	*/
	T & at(const Key &key) {
		map_iter it = mp.find(key);
		if(it == mp.end()) throw "invalid";
		list_iter pos = it->second;
		return pos->second;
	}
	const T & at(const Key &key) const {
		map_iter it = mp.find(key);
		if(it == mp.end()) throw "invalid";
		list_iter pos = it->second;
		return pos->second;
	}
	T & operator[](const Key &key) {
		return at(key);
	}
	const T & operator[](const Key &key) const {
		return at(key);
	}

	/**
	 * return an iterator point to the first 
	 * inserted and existed element
	 */
	iterator begin() {
		return iterator(&lst, lst.begin());
	}
	const_iterator cbegin() {
		return const_iterator(&lst, lst.begin());
	}
    /**
	 * return an iterator after the last inserted element
	 */
	iterator end() {
		return iterator(&lst, lst.end());
	}
	const_iterator cend() {
		return const_iterator(&lst, lst.end());
	}
  	/**	
	 * if didn't contain anything, return true, 
	 * otherwise false.
	 */
	bool empty() {
		return lst.empty();
	}

    void clear(){
		lst.destroy();
		mp.clear();
	}

	size_t size() const {
		return mp.siz;
	}
 	/**
	 * insert the value_piar
	 * if the key of the value_pair exists in the map
	 * update the value instead of adding a new elementï¼?
     * then the order of the element moved from inner of the 
     * list to the head of the list
	 * and return false
	 * if the key of the value_pair doesn't exist in the map
	 * add a new element and return true
	*/
	pair<iterator, bool> insert(const value_type &value) {
		map_iter pos_iter = mp.find(value.first);

		if(pos_iter == mp.end()) {
			lst.insert_tail(value);
			auto posi = lst.end();
			posi--;
			mp.insert(map_pair{value.first, posi});
			return pair<iterator, bool>{iterator(&lst, posi), true};
		}
		list_iter it = (pos_iter)->second;
		lst.erase(it);
		lst.insert_tail(value);
		auto posi = lst.end();
		posi--;
		mp.insert(map_pair{value.first,posi});
		return pair<iterator, bool>{iterator(&lst, posi), false};
	}
 	/**
	 * erase the value_pair pointed by the iterator
	 * if the iterator points to nothing
	 * throw 
	*/
	void remove(iterator pos) {
		list_iter now = pos.pos;
		if(now == lst.end())
			throw "invalid";
		mp.remove(now->first);
		lst.erase(now);
	}
	/**
	 * return how many value_pairs consist of key
	 * this should only return 0 or 1
	*/
	size_t count(const Key &key) const {
		if(mp.find(key) != mp.end())
			return 1;
		return 0;
	}
	/**
	 * find the iterator points at the value_pair
	 * which consist of key
	 * if not find, return the iterator 
	 * point at nothing
	*/
	iterator find(const Key &key) {
		map_iter it = mp.find(key);
		if(it == mp.end()) return end();
		return iterator(&lst, it->second);	
	}

};

class lru{
    
public:
	using lmap = sjtu::linked_hashmap<Integer,Matrix<int>,Hash,Equal>;
    using value_type = sjtu::pair<const Integer, Matrix<int> >;
	lmap mp;
	size_t capacity;

    lru(int size){
		capacity = size;
	}
    ~lru(){
    }
    /**
     * save the value_pair in the memory
     * delete something in the memory if necessary
    */
    void save(const value_type &v){
		mp.insert(v);
		if(mp.size() > capacity) {
			mp.remove(mp.begin());
		}
	}
    /**
     * return a pointer contain the value
    */
    Matrix<int>* get(const Integer &v){
		auto it = mp.find(v);
		if(it != mp.end()) {
			mp.insert(value_type(it->first, it->second));
			return &(it->second);
		}
		return nullptr;
    }
    /**
     * just print everything in the memory
     * to debug or test.
     * this operation follows the order, but don't
     * change the order.
    */
    void print(){
		for(auto u : mp) {
			std::cout << u.first.val << " " << u.second << std::endl;	
		}
    }
};
}

#endif