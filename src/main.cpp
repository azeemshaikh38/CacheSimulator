#include <iostream>
using namespace std;
#include <cstdio>
#include "../include/typedef.h"

#define CACHE_SIZE 8192
#define BLOCK_SIZE 16
#define ASSOCIATIVITY 1
#define WBWA 1
#define FILENAME "tracefile_2.txt"

int main() {
	cache L2_cache(CACHE_SIZE, BLOCK_SIZE, ASSOCIATIVITY, WBWA, NULL);
	cache L1_cache(CACHE_SIZE, BLOCK_SIZE, ASSOCIATIVITY, WBWA, &L2_cache);
 
	int L1_reads = 0, L1_read_misses = 0, L1_writes = 0, L1_write_misses = 0;	
	int L1_misses = 0, L2_misses = 0, data_access = 0, data_misses = 0;
	int access_type, ret;
	long int addr;
	block *evicted_block_L1, *evicted_block_L2;
	FILE *fp;

	fp = fopen(FILENAME, "r");
	while(fscanf(fp, "%d %x", &access_type, &addr) != EOF) {
		switch (access_type) {
			case 0: 									//Read Address
				data_access++;
				std::cout << "Type: Load. Addr: " << std::hex << addr << std::endl;
				L1_cache.read(addr, &ret);
				L1_reads++;
				if (!(ret)) {
					L1_read_misses++;
					data_misses++;
				}
				break;
			case 1:										//Write Address	
				data_access++;
				std::cout << "Type: Store. Addr: " << std::hex << addr << std::endl;
				L1_cache.write(addr, 0, &ret);
				L1_writes++;
				if (!(ret)) {
					L1_write_misses++;
					data_misses++;
				}
				break;
			default:									//Default
				std::cout << "Type: Unknown. Skip." << std::endl;
				break;
   		}
	}
	std::cout << "Data Access is: " << std::dec << data_access << " Data misses is: " << std::dec << data_misses << std::endl;	
	std::cout << "L1 reads: " << std::dec << L1_reads << " L1_read_misses: " << L1_read_misses << " L1 hit ratio: " << std::dec << (L1_reads - L1_read_misses)/(L1_reads) << std::endl;
	std::cout << "L1 writes: " << std::dec << L1_writes << " L1_write_misses: " << L1_write_misses << " L1 hit ratio: " << std::dec << (L1_writes - L1_write_misses)/(L1_writes) << std::endl;  
	return 0;
}
