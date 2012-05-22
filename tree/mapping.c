#include "mapping.h"
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/vmalloc.h>

MODULE_LICENSE("Dual BSD/GPL");

static node *mapping = NULL;

void add_node(uint64_t lba_hdd, uint64_t lba_ssd) {
    printk(KERN_WARNING "!!");
    node *s;
    s = vmalloc(sizeof(node));
    s->lba_ssd = lba_ssd;
    s->lba_hdd = lba_hdd;
    HASH_ADD_INT(mapping, lba_hdd, s); 
}

void del_node(node *n) {
    printk(KERN_WARNING "!!");
    HASH_DEL(mapping, n);
    vfree(n); 
}

node* find_item (uint64_t lba_hdd){ 
    printk(KERN_WARNING "!!");
    node *s;
    HASH_FIND_INT(mapping, &lba_hdd, s);  /* s: output pointer */
    return s;
}

void delete_all(void) {
    printk(KERN_WARNING "!!");
  node *current_node, *tmp;
  HASH_ITER(hh, mapping, current_node, tmp) {
    HASH_DEL(mapping,current_node);  /* delete it (users advances to next) */
    vfree(current_node);            /* free it */
  }
}


