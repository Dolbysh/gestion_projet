#include "uthash.h"
#include <linux/types.h>

typedef struct NODE{
	uint64_t lba_ssd;
	uint64_t lba_hdd;
	UT_hash_handle hh;
} node ;

void add_node(uint64_t lba_hdd, uint64_t lba_ssd);
void del_node(node *n);
void delete_all(void);
node* find_item(uint64_t lba_hdd);
