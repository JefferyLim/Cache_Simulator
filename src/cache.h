/***************************************
Jeffery Lim
Taylor Nguyen
****************************************/
#ifndef CACHE_H
#define CACHE_H

#include "lru.h"


//We had this like this because set had more stuff and we reduced it a lot
struct set
{
	lru* blocks;
};

class cache
{
	private:
		unsigned int block_size;
		unsigned int cache_size;
		unsigned int indexes;
		unsigned int ways;
		unsigned int hit_time;
		unsigned int miss_time;
		
		unsigned int transfer_time;
		unsigned int bus_width;
				
		set* cache_index;
		set* victim_cache;
		cache* transfer_level;
	public:
		
		unsigned int get_block_size();
		unsigned int get_cache_size();
		unsigned int get_indexes();
		unsigned int get_ways();
		unsigned int get_hit_time();
		unsigned int get_miss_time();

		unsigned long long int get_address(unsigned long long int tag, unsigned long long int index);
		unsigned long long int get_tag(unsigned long long int address);
		unsigned long long int get_index(unsigned long long int address);

		unsigned long long int hit_count;
		unsigned long long int vc_hit;
		unsigned long long int miss_count;
		unsigned long long int request_count;
		unsigned long long int kickout_count;
		unsigned long long int dirty_kickout;
		unsigned long long int transfer_count;

		cache();
		void cache_deconstruct();
		void set_cache(unsigned int bs, unsigned int cs, unsigned int assoc, unsigned int ht, unsigned int mt, unsigned int tt, unsigned int bw, cache* transfer);
		unsigned long long int find(unsigned long long int address, char op);

		void print();
	
};

#endif