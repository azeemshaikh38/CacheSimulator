#include <iostream>
#include "../include/typedef.h"
using namespace std;


/**************************************Cache Initialization************************************/
cache::cache(int cache_size_in, int block_size_in, int assoc_in, int WBWA_in, cache *lower_cache_in) {
	cache_size = cache_size_in;
	block_size = block_size_in;
	assoc = assoc_in;
/***********************************Find number of sets in Cache*******************************************/
	num_of_sets = cache_size/(block_size*assoc);

	std::cout << "Initialized cache with cache size " << std::dec << cache_size << 
	" block size " << std::dec << block_size << " associativity " << std::dec << 
	assoc << " number of sets " << std::dec << num_of_sets << std::endl;

/*******************Create num_of_sets number of array and return array pointer*******************/
	head = (sethead *)malloc(sizeof(sethead)*num_of_sets);
	int i;
	for (i = 0; i < num_of_sets; i++) {
		head[i].capacity = 0;
		head[i].ll = NULL; 
	}

/**********************************Find number of bits for offset******************************************/
	int x = block_size, offset = 0;
	while(x != 0) {
		offset++;
		x = x>>1;
	}
	offset_bits = offset - 1;

/**************************************Find number of bits for set index*************************************/
	x = num_of_sets; offset = 0;
	while(x != 0) {
		offset++;
		x = x>>1;
	}
	index_bits = offset - 1;

	std::cout << "Number of bits for offset " << std::dec << offset_bits << 
	" number of bits for index " << std::dec << index_bits << std::endl;

/**********************************************Write Policies*************************************************/
	if (WBWA_in) {
		WBWA = true;
	} else {
		WBWA = false;
	}
	
/*************************************************Lower Level Memory******************************************/
	lower_cache = lower_cache_in;
}

/***************************************************Read Cache************************************************/
int cache::read(long int read_addr, int *hit) {
	long int tag_tmp = read_addr >> (index_bits + offset_bits);
	int index = (read_addr ^ (tag_tmp << (index_bits + offset_bits))) >> offset_bits;
	int data, search_hit;
	block *evicted_block;

	std::cout << "Read: addr " << std::hex << read_addr << " Tag " << 
	std::hex << tag_tmp << " Index " << std::dec << index << std::endl;

	data = cache::search_tag(tag_tmp, &head[index], hit);
	if (*hit) {
		return data;
	} 

	/******************************Cache Miss*************************/

	if (lower_cache != NULL) {	
		data = lower_cache->read(read_addr, &search_hit);
	}

	evicted_block = cache::update(read_addr, data);
	if ((WBWA) && (evicted_block != NULL)) {
		if ((evicted_block->dirty) && (lower_cache != NULL)) {
			lower_cache->write(evicted_block->addr, evicted_block->data, &search_hit);
		}
	}
	
	return data;
}

/***************************************************Write Cache**********************************************/
int cache::write(long int write_addr, int data, int *hit) {
	
	int search_hit;
	block * evicted_block;

	cache::invalidate(write_addr, data, hit);
	
	if (WBWA) {
		if (!(*hit)) {
			if (lower_cache != NULL) {
				data = lower_cache->read(write_addr, &search_hit);
			}
			evicted_block = cache::update(write_addr, data);
			if (evicted_block != NULL) {
				if ((evicted_block->dirty) && (lower_cache != NULL)) {
					lower_cache->write(evicted_block->addr, evicted_block->data, &search_hit);
				}
			}
			cache::invalidate(write_addr, data, &search_hit);
		}
		return 0;
	}

	lower_cache->write(write_addr, data, &search_hit);	
	return 0;
	
}

/****************************************************Cache Update**********************************************/
block * cache::update(long int addr, int data) {
	long int tag = addr >> (index_bits + offset_bits);
	int index = (addr ^ (tag << (index_bits + offset_bits))) >> (offset_bits);
	std::cout << "Update: addr " << std::hex << addr << " Tag " << std::hex << tag << " Index " << std::dec << index << std::endl;
	
	if (head[index].capacity < assoc) {
		block *tmp = (block *)malloc(sizeof(block));
		tmp->tag = tag;
		tmp->valid = true;
		tmp->dirty = false;
		tmp->addr = addr;
		tmp->prev = NULL;
		tmp->next = head[index].ll;
		head[index].capacity++;
		head[index].ll = tmp;
		return NULL;
	} else {
		block *tmp = head[index].ll;
		block *tmp_prev = NULL;
		while(tmp != NULL) {
			tmp = tmp->next;
			tmp_prev = tmp;
		}
				
		if(tmp_prev->prev != NULL) {		//Associativity = 1. Direct Mapped
			(tmp_prev->prev)->next = NULL;
   		}

		tmp = (block *)malloc(sizeof(block));
		tmp->tag = tag;
		tmp->valid = true;
		tmp->dirty = false;
		tmp->addr = addr;
		tmp->next = head[index].ll;
		tmp->prev = NULL;
		head[index].ll = tmp;
 
		return tmp_prev;
	}	
}

/*****************************************************Invalidate bit******************************************/
void cache::invalidate(long int addr, int data, int *hit) {
	long int tag = addr >> (index_bits + offset_bits);
	int index = (addr ^ (tag << (index_bits + offset_bits))) >> (offset_bits);

	block *tmp = head[index].ll;
	while(tmp != NULL) {
		if(tmp->tag == tag) {
			if(tmp->prev != NULL) {
				(tmp->prev)->next = tmp->next;
			}
			if (tmp->next != NULL) {
				(tmp->next)->prev = tmp->prev;
			}
			tmp->next = head->ll;
			tmp->prev = NULL;
			head->ll = tmp;
			tmp->valid = false;
			tmp->dirty = true;
			tmp->data = data;
			*hit = 1;
			std::cout << "Invalidated!!" << std::endl;
			return;	
		}
		tmp = (tmp->next);
	}
	*hit = 0; 
	std::cout << "Invalidate Cache Miss!!" << std::endl;
	return;
}

/**************************************************Search Tag**************************************/
int cache::search_tag(long int tag, sethead *head, int *hit) {
	block *tmp = head->ll;
	while(tmp != NULL) {
		if(tmp->tag == tag) {
			/*****************Update block according to LRU*****************/
			if(tmp->prev != NULL) {
				(tmp->prev)->next = tmp->next;
			}
			if (tmp->next != NULL) {
				(tmp->next)->prev = tmp->prev;
			}
			tmp->next = head->ll;
			tmp->prev = NULL;
			head->ll = tmp;
			std::cout << "Cache Hit!!!" << std::endl;
			*hit = 1;
			return tmp->data;
		}
		tmp = (tmp->next);
	} 
	std::cout << "Cache Miss!!" << std::endl;
	*hit = 0;
	return -1;
}
/*
int main() {
	return 0;
}
*/
