/***************************************
Jeffery Lim
Taylor Nguyen
****************************************/
#include <math.h>
#include <iostream>

#include "cache.h"

using namespace std;

/************************************
cache::cache()

Initailizes the counters of the cache
The actual cache initialization is in set cache
*************************************/
cache::cache(){
	//initializes the counters
	hit_count = 0;
	miss_count = 0;
	request_count = 0;
	kickout_count = 0;
	dirty_kickout = 0;
	transfer_count = 0;
	vc_hit = 0;
}

/************************************
void cache::set_cache(unsigned int bs, unsigned int cs, unsigned int assoc, unsigned int ht, unsigned int mt, unsigned int tt, unsigned int bw,cache* transfer)
	
Initializes all the cache parameters
bs -> blocksize
cs -> cache size
assoc -> ways
ht -> hit time
mt -> miss time
tt -> transfer time
bw -> bus width
transfer level -> next level of memory
*************************************/
void cache::set_cache(unsigned int bs, unsigned int cs, unsigned int assoc, unsigned int ht, unsigned int mt, unsigned int tt, unsigned int bw,cache* transfer){
	block_size = bs;
	cache_size = cs;

	indexes = (cs/bs/assoc); 
	ways = assoc;
	hit_time = ht;
	miss_time = mt;
	transfer_time = tt;
	bus_width = bw;
	transfer_level = transfer;
	
	//Sets memory for the LRU and set

	cache_index = new set[indexes];
	for (unsigned int i = 0; i < indexes; i++)
	{	
		cache_index[i].blocks = new lru(ways);
	}
	//Sets memory for the victim cache
	victim_cache = new set;
	victim_cache->blocks = new lru(8);

	return;
		
}

/************************************
void cache::cache_deconstruct()

Deconstructs the cache
*************************************/
void cache::cache_deconstruct(){
	for(unsigned int i = 0; i < indexes; i++){
		cache_index[i].blocks->lru_deconstruct();
	}
	delete cache_index;
	
	victim_cache->blocks->lru_deconstruct();
	delete victim_cache;
	return;
}

/************************************
unsigned long long int cache::find(unsigned long long int address, char op)
	
Returns cycles

Finds the address in the cache and will either write or read
In this simulator, the only difference between write and read is that the write operation marks a block dirty
*************************************/
unsigned long long int cache::find(unsigned long long int address, char op){
	unsigned long long int execution = 0;
	unsigned long long int tag = 0;
	unsigned long long int vc_tag = 0;
	unsigned long long int temp = 0;
	unsigned long int index = 0;
	
	//get the tag and index from address and cache parameters
	tag = get_tag(address);		
	index = get_index(address);

	node* c_ptr; //cache pointer
	node* v_ptr; //victim pointer

	request_count++;
	
	c_ptr = cache_index[index].blocks->get_head(); 	//get the head of cache

	//Look for the specific node with the right tag in index
	while(c_ptr->next != NULL){						
		c_ptr = c_ptr->next;
		if(c_ptr->valid && c_ptr->tag == tag){			//Check the node's valid and tag
			//Update, Hit, return											
			cache_index[index].blocks->update(c_ptr);	
			hit_count++;
			execution += hit_time;
			if(op == 'W'){
				c_ptr->dirty = 1;
			}
			return execution;
		}	
	}
	
	//Miss
	miss_count++;		
	execution += miss_time;
	
	vc_tag = get_address(tag, index);		//Get the address from the tag

	v_ptr = victim_cache->blocks->get_head();	//Get the head of the victim cache
	while(v_ptr->next != NULL){				
		v_ptr = v_ptr->next;
		if(v_ptr->valid && v_ptr->tag == vc_tag){		
		//VC Hit
			c_ptr = cache_index[index].blocks->get_tail();	
			vc_hit++;	
			execution += hit_time;
			if(op == 'W'){
				v_ptr->dirty = 1;
			}	
			
			//Swap the values of the victim hit and the cache eviction
			temp = v_ptr->dirty;
			v_ptr->dirty = c_ptr->dirty;
			c_ptr->dirty = temp;

			temp = v_ptr->valid;
			v_ptr->valid = c_ptr->valid;
			c_ptr->valid = temp;

			temp = v_ptr->tag;
			v_ptr->tag = get_address(c_ptr->tag,  index);
			c_ptr->tag = get_tag(temp);
			
			cache_index[index].blocks->update(c_ptr);
			victim_cache->blocks->update(v_ptr);
		
			return execution;
		}
	}

	v_ptr = victim_cache->blocks->get_tail();			//Get the victim set and the cache set tail
	c_ptr = cache_index[index].blocks->get_tail();
	
	if(c_ptr->valid == 1){				//Check if the victim cache is full i.e tail is valid 
		if(v_ptr->valid == 1){			//Check if the cache is full 		i.e tail is valid	
			kickout_count++;	
			if(v_ptr->dirty == 1){		//If the victim eviction is dirty, we need to perform dirty kickout	
				dirty_kickout++;
				if(transfer_level != NULL){									//Find the address from next level of memory
					execution += transfer_level->find(v_ptr->tag, 'W');		//v_ptr->tag is full address
					execution += transfer_time*(block_size/bus_width);
				}else if(transfer_level == NULL){		//Go to main memory
					execution += 10+50+(15*64/bus_width);
				}
			}
		}
		
		//Insert the LRU Cache block
		v_ptr->tag = get_address(c_ptr->tag,index);		
		v_ptr->valid = c_ptr->valid;
		v_ptr->dirty = c_ptr->dirty;
		victim_cache->blocks->update(v_ptr);
		
	}
	
	transfer_count++;
	if(transfer_level != NULL){								//Go to the next level cache to retrieve data
		execution+= transfer_level->find(address, 'R');		
		execution+= transfer_time*(block_size/bus_width);
	}else if(transfer_level == NULL){
		execution += 10+50+(15*64/bus_width);
		
	}
	
	//Insert the new data into the main cache
	c_ptr->tag = tag;		
	c_ptr->valid = 1;
	c_ptr->dirty = 0;
	if(op == 'W'){
		c_ptr->dirty = 1;	
	}

	cache_index[index].blocks->update(c_ptr);
	execution += hit_time;
	return execution;

}


unsigned int cache::get_block_size(){
	return block_size;
}
unsigned int cache::get_cache_size(){
	return cache_size;
}
unsigned int cache::get_indexes(){
	return indexes;
}
unsigned int cache::get_ways(){
	return ways;
}
unsigned int cache::get_hit_time(){
	return hit_time;
}
unsigned int cache::get_miss_time(){
	return miss_time;
}


/************************************
unsigned long long int get_index(unsigned long long int address)

Returns the index from the address
*************************************/
unsigned long long int cache::get_index(unsigned long long int address){
	unsigned long long int temp;
	unsigned long long int index;
	temp = 64 - log2(get_indexes())-log2(get_block_size()); 	//Gives us the size of tag
	index = address << temp;									//Zero out the top bits
	index = index >> temp;
	temp = log2(get_block_size());							   	//Shifting it by log2(bytesize) gives us the index size
	index = index >> temp;
	return index;
}
/************************************
unsigned long long int get_tag(unsigned long long int address)

Returns the tag from the address
*************************************/
unsigned long long int  cache::get_tag(unsigned long long int address){
	unsigned long long int temp;
	unsigned long long int tag;
	temp = log2(get_block_size());			//Shift it right to get rid of the byteoffset
	tag = address >> temp;
	temp = log2(get_indexes());			//Shift it right to get rid of index
	tag = tag >> temp;	
	return tag;

}
/************************************
unsigned long long int get_address(unsigned long long int tag,	 unsigned long long int index){

Reconstructs the address from the tag and index
*************************************/
unsigned long long int  cache::get_address(unsigned long long int tag, unsigned long long int index){
	unsigned long long int temp;
	unsigned long long int address = 0;

	temp = log2(get_indexes());	
	address = tag << temp;					//Shift tag by index size
	address = address | index;				//Or in the index so its Tag Index
	temp = log2(get_block_size());			//Shift it left by byte offset size
	address = address << temp;
	return address;
}

unsigned long long int log2(unsigned long long int number){
	return (unsigned int)round(log(number)/log(2));
}

void cache::print(){
	unsigned int i;
	node * temp;
	for(i = 0; i < indexes; i++){
		temp = cache_index[i].blocks->get_head();
			while(temp->next != NULL){
				temp = temp->next;
				cout << hex << i << ": " << "V: " << temp->valid << " D: " << temp->dirty << " tag: " <<hex << temp->tag << " | ";
			}
			cout << endl;
	}
	cout << endl;

	cout << "\nVictim Cache: \n"<< endl;
	temp = victim_cache->blocks->get_head();

 	i = 0;
	while(temp->next != NULL){
		temp = temp->next;
		cout << hex << i << ": " << "V: " << temp->valid << " D: " << temp->dirty << " Addr: " <<hex << temp->tag << endl;
		i++;
	}
}


