#include "mapping.h"
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/vmalloc.h>

MODULE_LICENSE("Dual BSD/GPL");

static node *mapping = NULL;
static node *mapping_ssd = NULL;

void add_node(sector_t lba_hdd, sector_t lba_ssd) {
    printk(KERN_WARNING "!!");
    node *s;
    s = vmalloc(sizeof(node));

    if(s == NULL){
        printk(KERN_WARNING "Could not allocate\n");
        return;
    }

    s->lba_ssd = lba_ssd;
    s->lba_hdd = lba_hdd;
    HASH_ADD(hh, mapping, lba_hdd, sizeof(sector_t), s); 
    HASH_ADD(hh1, mapping_ssd, lba_ssd, sizeof(sector_t), s); 
}

void del_node(node *n) {
    printk(KERN_WARNING "!!");
    HASH_DEL(mapping, n);
    vfree(n); 
}

node* find_item (sector_t lba_hdd){ 
    printk(KERN_WARNING "!!");
    node *s;
    HASH_FIND(hh, mapping, &lba_hdd, sizeof(sector_t), s);  /* s: output pointer */
    return s;
}

node* find_item_ssd(sector_t lba_ssd){
    printk(KERN_WARNING "!!");
    node *s;
    HASH_FIND(hh1, mapping, &lba_ssd, sizeof(sector_t), s);  /* s: output pointer */
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


