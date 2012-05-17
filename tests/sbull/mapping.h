#include "uthash.c"

typedef struct NODE{
	int lba_ssd;
	int lba_hdd;
	UT_hash_handle hh;
} node ;

void add_node(int lba_ssd, int lba_hdd);
void del_node(int lba_ssd);
node* find_item(int lba_ssd);


