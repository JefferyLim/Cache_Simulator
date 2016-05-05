#ifndef LRU_H
#define LRU_H

struct node {
	node* next;
	node* prev;
	bool dirty;
	bool valid;
	unsigned long long int tag;
}; 

class lru {
private:
	struct node* head;
	struct node* tail; 

public:
	lru();
	lru(unsigned int way);
	void lru_deconstruct(); 

	node* get_tail();
	node* get_head(); 

	void update(node* ptr);
	void print();
}; 

#endif
