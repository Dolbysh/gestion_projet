#include "mapping.h"
#include <stdlib.h>  /* atoi, malloc */

static node *mapping = NULL;

void add_node(uint64_t lba_hdd, uint64_t lba_ssd) {
    node *s;
    s = malloc(sizeof(node));
    s->lba_ssd = lba_ssd;
    s->lba_hdd = lba_hdd;
    HASH_ADD_INT(mapping, lba_hdd, s); 
}

void del_node(node *n) {
    HASH_DEL(mapping, n);
    free(n); 
}

node* find_item (uint64_t lba_hdd){ 
    node *s;
    HASH_FIND_INT(mapping, &lba_hdd, s);  /* s: output pointer */
    return s;
}

void delete_all() {
  node *current_node, *tmp;
  HASH_ITER(hh, mapping, current_node, tmp) {
    HASH_DEL(mapping,current_node);  /* delete it (users advances to next) */
    free(current_node);            /* free it */
  }
}


