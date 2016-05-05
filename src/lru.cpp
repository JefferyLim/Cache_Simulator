/***************************************
Jeffery Lim
Taylor Nguyen
****************************************/
#include <iostream>
#include <stdio.h>

#include "lru.h"

using namespace std;


lru::lru(){
	
}

/************************************
lru::lru(unsigned int way)

Initalizes the lru with way number of nodes
*************************************/
lru::lru(unsigned int way){
    node* temp = new node;
    head = temp;

	node* temp1 = temp;
	
	for(unsigned int i = 0; i < way; i++){
		node* entry = new node;
		temp->next = entry;
		temp = temp->next;
		temp->prev = temp1;
		temp->tag = 0;
		temp->dirty = 0;
		temp->valid = 0;
		temp1 = temp;		
	}

	tail = temp;
	temp->next = NULL;

}


/************************************
void lru::lru_deconstruct()

Deconstructs the LRU
*************************************/
void lru::lru_deconstruct(){

	node*temp = head;

	while(head != NULL){
		head = head->next;
		delete temp;
		temp = head;
	}

	delete temp;
	return;
}

/************************************
node* lru::get_tail()

Returns the tail
*************************************/
node* lru::get_tail(){
	return tail;
}


/************************************
node* lru::get_head()

Returns the head
*************************************/
node* lru::get_head(){
	return head;
}

/************************************
void lru::update(node* ptr)

Updates the linked list by bringing the ptr to the head of the list
*************************************/
void lru::update(node* ptr){
	if(ptr->valid == 0 || head->next == ptr){
		return;
	}
	
	node* temp = head;
	temp = head->next;
	(ptr->prev)->next = ptr->next;

	if(tail == ptr){
		tail = (ptr->prev);
	}else{
		(ptr->next)->prev = ptr->prev;
	}

	temp->prev = ptr;
	ptr->next = temp;
	ptr->prev = NULL;
	head->next = ptr;

	return;

}

void lru::print(){

	node* temp = head;	
	int i = 0;
	while(temp->next != NULL ){
			temp = temp->next;
			cout << hex << i << ": " << "Valid: " << temp->valid << " Dirty: " << temp->dirty << " Tag: " <<hex << temp->tag << endl;
			i++;
	}
	if(i == 0){
		cout << hex << i << ": " << "Valid: " << temp->valid << " Dirty: " << temp->dirty << " Tag: " <<hex << temp->tag << endl;
	}
	
	cout << endl;
	
}
	