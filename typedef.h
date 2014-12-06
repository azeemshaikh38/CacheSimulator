typedef struct block {
        int tag;
        bool valid;
	bool dirty;
	long int addr;
	int data;
        block *next;
        block *prev;
};

typedef struct sethead {
        block *ll;
        int capacity;
};

class cache {
        public:
                cache(int, int, int, int, cache *);   //Cache size, Block size, Associativity, WBWA, Lower Level Cache
                int read(long int, int *);          //Read Address, Hit
                int write(long int, int, int *);    //Write Address, Data, Hit
                block * update(long int, int);       //Address, Data
		void invalidate(long int, int, int *);	//Address
        private:
                int search_tag(long int, sethead *, int *); //Tag, Head struct, Hit
                int cache_size, block_size, assoc, num_of_sets, index_bits, offset_bits;
                sethead *head;
		bool WBWA;
		cache *lower_cache;
};
