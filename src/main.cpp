/***************************************
Jeffery Lim
Taylor Nguyen
****************************************/
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <math.h>

#include "lru.h"
#include "cache.h"

using namespace std;

void readConfig(char* file, cache* L1_I, cache* L1_D, cache* L2);	
void readTrace(cache* L1_I, cache* L1_D);
void printReport(char* sim, char* config, cache* L1_I, cache* L1_D, cache* L2);

unsigned long long int instruction_count= 0; 	//number of instructions
unsigned long long int read_count= 0; 			//number of reads 
unsigned long long int write_count= 0; 			//number of writes 

unsigned long long int read_time= 0;			//the amount of read time in cycles
unsigned long long int write_time= 0;			//the amount of write time in cycles
unsigned long long int inst_time= 0;			// the amount of instruction time in cycles

unsigned int chunksize = 8;						//main memory chunk size. 

int main(int argc, char *argv[]){
	
	//Summon the three caches
	cache* L1_D = new cache;
	cache* L1_I = new cache;
	cache* L2 	= new cache;

	cout << "Begin Configurations" << endl;	

	//argc == 3 means that there is a config file; otherwise, use the defaults 
	//Normally it should be argc == 2, but since we did not want to mess with writing shell scripts
	//argc == 3 because we included the trace file name. This makes writing the report easier.
	if(argc == 3){
		cout << "Reading config file" << endl;
		readConfig(argv[2], L1_I, L1_D, L2);
	}else{
		//no argument means we are using default case
		L1_I->set_cache(32, 8192, 1, 1, 1, 10, 16, L2);
		L1_D->set_cache(32, 8192, 1, 1, 1, 10, 16, L2);
		L2->set_cache(64, 32768, 1, 8, 10, 15, 8, NULL); //Null means Main Memory
	} 

	cout << "Reading Traces" << endl;
	readTrace(L1_I, L1_D);
	
	/*
	cout << "L1_I:" <<endl;
	L1_I->print();	
	cout << "L1_D:" <<endl;
	L1_D->print();	
	cout << "L2:" <<endl;
	L2->print();	
	*/

	cout << "Printing Report" << endl;
	printReport(argv[1], argv[2], L1_I, L1_D, L2);	//Print out the report to an external file
	
	cout <<"Deconstructing Caches" << endl;
	L1_D->cache_deconstruct();
	L1_I->cache_deconstruct();
	L2->cache_deconstruct();
	
	delete L1_D;
	delete L1_I;
	delete L2;
	
	cout <<"Finished";
	return 0;
}
	
/************************************
void readConfig(char* file, cache* L1_I, cache* L1_D, cache* L2)

char*file is the config file config
cache* is the pointer to point to the three caches

This function reads the config files and sets the cache parameters with the proper values

*************************************/
void readConfig(char* file, cache* L1_I, cache* L1_D, cache* L2){
	unsigned int bs = 0, bs1 = 0; // block size
	unsigned int cs = 0, cs1 = 0; // cache sinceze 
	unsigned int assoc = 0, assoc1 = 0; // ways
	
	char input[20];
	unsigned int value;
	FILE *config = fopen(file, "r");
	
	if(!config){
		L1_I->set_cache(32,8192, 1, 1, 1, 10, 16, L2);
		L1_D->set_cache(32,8192, 1, 1, 1, 10, 16, L2);																																								
		L2->set_cache(64, 32768, 1, 8, 10, 15, 8, NULL); //Null means Main Memory
		return;
	 }else{

		 while (fscanf(config,"%s %d\n", input, &value) == 2) {
			 if(strcmp(input, "L1_block_size") == 0){
		 		bs = value;
			 }else if(strcmp(input, "L1_cache_size") == 0){
		 		cs = value;
			 }else if(strcmp(input, "L1_assoc") == 0){
		 		assoc = value;
			 }

			if(strcmp(input, "L2_block_size") == 0){
		 		bs1 = value;
			 }else if(strcmp(input, "L2_cache_size") == 0){
		 		cs1 = value;
			 }else if(strcmp(input, "L2_assoc") == 0){
		 		assoc1 = value;
			 }
			 
			 if(strcmp(input, "mem_chunksize") == 0){
		 		chunksize = value;
			 }
		}

		L1_I->set_cache(bs,cs, assoc, 1, 1, 10, 16, L2);
		L1_D->set_cache(bs,cs, assoc, 1, 1, 10, 16, L2);
		L2->set_cache(bs1,cs1, assoc1, 8, 10, 15, chunksize, NULL);
		
	}
}	

	
/************************************
void readTrace(cache* L1_I, cache* L1_D)

cache* is the pointer to point to the two L1 caches

This function reads the trace files, figures out the number of requests needed to be made and then proceeds to call the 
necessary find function
*************************************/
void readTrace(cache* L1_I, cache* L1_D){
	char op;
    unsigned long long int address;
    unsigned int bytesize;
	unsigned int references;
	
	//scanf the lines by op code, address, and byte
	while(scanf("%c %Lx %d\n", &op, &address, &bytesize) == 3)
	{	
		references = (address%4 + bytesize-1)/4 + 1;	//Address%4 gives us location of the start of block
														//Adding it to bytesize-1 gives us the number of total bytes
														//Dividing it by 4 gives us the number of references needed
														//ex: 0003 with 6 bytesize means we want to start at 0003 and end 0009
														//0003%4 = 3 + 6 - 1 = 8/2 = 2 + 1 = 3 references
		switch(op){
			case 'I':
				instruction_count++;
				inst_time += 1;		//Execute cycle time
				for(unsigned int i = 0; i < references; i++){			
					inst_time += L1_I->find(address + i*4, op);
				}

				break;
			case 'R':
				read_count++;
				for(unsigned int i = 0; i < references; i++){			
					read_time += L1_D->find(address + i*4, op);
				}

				break;
			case 'W':
				write_count++;	
				for(unsigned int i = 0; i < references; i++){			
					write_time += L1_D->find(address + i*4, op);
				}	
					
				break;
			default:
				break;
		}		
	}
}
/************************************
void printReport(char* sim, char* config, cache* L1_I, cache* L1_D,cache* L2)

char* sim is the trace file config
char* config is the name of the config file
cache* is the pointer to point to the two L1 caches

This function reads the trace files, figures out the number of requests needed to be made and then proceeds to call the 
necessary find function
*************************************/
void printReport(char* sim, char* config, cache* L1_I, cache* L1_D,cache* L2){
	FILE * report;

	char result[50];
	//default case will be empty
	
	//copy the string into results and concatentate teh rest of the names into it
	strcpy(result, sim);
	strcat(result, ".");
	if(config == NULL){
		config = "default";
	}
	strcat(result, config);
	report = fopen(result, "w");

	unsigned long long int execution = inst_time + read_time + write_time; //calculate the execution time
	unsigned long long int total = read_count+write_count+instruction_count;
	unsigned long long int L1Cost = L1_I->get_cache_size()/4096*100+L1_I->get_cache_size()/4096*(log2(L1_I->get_ways()))*100;
	unsigned long long int L1Cost1 = L1_D->get_cache_size()/4096*100+L1_D->get_cache_size()/4096*(log2(L1_D->get_ways()))*100;
	unsigned long long int L2Cost = L2->get_cache_size()/16384*50+ L2->get_cache_size()/16384*(log2(L2->get_ways()))*50;
	unsigned long long int memCost = 50+25+100*(log2(chunksize)-log2(8));

	fprintf(report, "\n-------------------------------------------------------------------------\n");
    fprintf(report, "      %s.%s       Simulation Results\n", sim, config);
    fprintf(report, "-------------------------------------------------------------------------\n\n");
	fprintf(report, "  Memory system: \n");
	fprintf(report, "    Dcache size = %u : ways = %u : block size = %u \n", L1_D->get_cache_size(), L1_D->get_ways(), L1_D->get_block_size());
	fprintf(report, "    Icache size = %u : ways = %u : block size = %u \n", L1_I->get_cache_size(), L1_I->get_ways(), L1_I->get_block_size());
	fprintf(report, "    L2-cache size = %u : ways = %u : block size = %u \n", L2->get_cache_size(), L2->get_ways(), L2->get_block_size());
	fprintf(report, "    Memory ready time = %u : chunksize = %u : chunktime = %u \n\n", 50, chunksize, 15);

	fprintf(report, "  Execute time =    %llu;  Total refs = %llu\n", execution, total);
	fprintf(report, "  Inst refs = %llu;  Data refs = %llu\n\n", instruction_count , write_count+read_count);

	fprintf(report, "  Number of reference types:  [Percentage]\n");
	fprintf(report, "    Reads  =%13llu    [%4.1f%%]\n", read_count, (double)read_count/(double)(total)*100);
	fprintf(report, "    Writes =%13llu    [%4.1f%%]\n", write_count, (double)write_count/(double)(total)*100);
	fprintf(report, "    Inst.  =%13llu    [%4.1f%%]\n", instruction_count, (double)instruction_count/(double)(total)*100);
	fprintf(report, "    Total  =%13llu\n\n", total);

	fprintf(report, "  Total cycles for activities:  [Percentage]\n");
	fprintf(report, "    Reads  =%13llu    [%4.1f%%]\n", read_time, (double)read_time/(double)(execution)*100);
	fprintf(report, "    Writes =%13llu    [%4.1f%%]\n", write_time, (double)write_time/(double)(execution)*100);
	fprintf(report, "    Inst.  =%13llu    [%4.1f%%]\n", inst_time, (double)inst_time/(double)(execution)*100);
	fprintf(report, "    Total  =%13llu\n\n", execution);

	fprintf(report, "  CPI = %.1f\n", (double)(execution)/instruction_count);
	fprintf(report, "  Ideal: Exec. Time = %llu; CPI =  %.1f\n", total+instruction_count, (double)(total+instruction_count)/instruction_count);
	fprintf(report, "  Ideal mis-aligned: Exec. Time = %llu; CPI =  %.1f\n\n", L1_I->request_count + L1_D->request_count + instruction_count ,(double)(L1_I->request_count + L1_D->request_count + instruction_count)/instruction_count);

	fprintf(report, "  Memory Level:  L1i\n");
	fprintf(report, "    Hit Count = %llu  Miss Count = %llu\n", L1_I->hit_count, L1_I->miss_count);
	fprintf(report, "    Total Requests = %llu\n", L1_I->request_count);
	fprintf(report, "    Hit Rate = %.1f%%   Miss Rate =  %.1f%%\n", (double)(L1_I->hit_count)/(double)(L1_I->hit_count+L1_I->miss_count)*100,(double)(L1_I->miss_count)/(double)(L1_I->request_count)*100);
	fprintf(report, "    Kickouts = %llu; Dirty kickouts = %llu; Transfers = %llu\n",  L1_I->kickout_count,  L1_I->dirty_kickout,  L1_I->transfer_count);
	fprintf(report, "    VC Hit count = %llu\n\n", L1_I->vc_hit);

	fprintf(report, "  Memory Level:  L1d\n");
	fprintf(report, "    Hit Count = %llu  Miss Count = %llu\n", L1_D->hit_count, L1_D->miss_count);
	fprintf(report, "    Total Requests = %llu\n", L1_D->request_count);
	fprintf(report, "    Hit Rate = %.1f%%   Miss Rate = %.1f%%\n", (double)(L1_D->hit_count)/(double)(L1_D->hit_count+L1_D->miss_count)*100,(double)(L1_D->miss_count)/(double)(L1_D->request_count)*100);
	fprintf(report, "    Kickouts = %llu; Dirty kickouts = %llu; Transfers = %llu\n",  L1_D->kickout_count,  L1_D->dirty_kickout,  L1_D->transfer_count);
	fprintf(report, "    VC Hit count = %llu\n\n", L1_D->vc_hit);

	fprintf(report, "  Memory Level:  L2\n");
	fprintf(report, "    Hit Count = %llu  Miss Count = %llu\n", L2->hit_count, L2->miss_count);
	fprintf(report, "    Total Requests = %llu\n", L2->request_count);
	fprintf(report, "    Hit Rate = %.1f%%   Miss Rate = %.1f%%\n", (double)(L2->hit_count)/(double)(L2->hit_count+L2->miss_count)*100,(double)(L2->miss_count)/(double)(L2->request_count)*100);
	fprintf(report, "    Kickouts = %llu; Dirty kickouts = %llu; Transfers = %llu\n",  L2->kickout_count,  L2->dirty_kickout,  L2->transfer_count);
	fprintf(report, "    VC Hit count = %llu\n\n", L2->vc_hit);

	fprintf(report, "  L1 cache cost (Icache $%llu) + (Dcache $%llu) = $%llu\n", L1Cost, L1Cost1, L1Cost+L1Cost1);
	fprintf(report, "  L2 cache cost = $%llu;  Memory cost = $%llu  Total cost = $%llu", L2Cost, memCost, L1Cost+L2Cost+memCost+L1Cost1);
	fclose(report);
	
}


