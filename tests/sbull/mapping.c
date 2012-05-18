#include "mapping.h"
#include <stdlib.h>  /* atoi, malloc */

struct node *mapping = NULL;

void add_node(int lba_ssd, int lba_hdd) {
    struct node *s;
    s = malloc(sizeof(struct node));
    s->lba_ssd = lba_ssd;
    s->lba_hdd = lba_hdd;
    HASH_ADD_INT( mapping, lba_ssd, s ); 
}

void del_node(struct node *n) {
    HASH_ADD_INT( mapping, n );
    free(n); 
}

node* find_item(int lba_ssd){ 
    struct my_struct *s;
    return HASH_FIND_INT( mapping, &lba_ssd, s );  /* s: output pointer */
    return s;
}

void delete_all() {
  struct node *current_node, *tmp;
  HASH_ITER(hh, mapping, current_node, tmp) {
    HASH_DEL(mapping,current_node);  /* delete it (users advances to next) */
    free(current_node);            /* free it */
  }
}


