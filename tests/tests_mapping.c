//#include "mapping.h"
#include "free_sectors.h"
#include <stdio.h>

extern free_sector *mem_SSD;

int main() {
		
	//tests free_sectors.c;
	init_free_sector(42, 70, 60, 10, 67);
//	free_sector *mem_SSD == NULL;
	if (mem_SSD != NULL)
		printf("mem_SSD->size = %llu", mem_SSD->size_line);
	else
		printf("Try again...!");
			
	/*
	if (mem_SSD != NULL) {
		printf("size : %llu", mem_SSD->size_line);
	}
	*/
	/* //tests mapping.c
	add_node(11, 1);
	add_node(22, 2);
	
	node *i1 = find_item(11);
	node *i2 = find_item(22);
	printf("OK\n");
	
	if (i1 != NULL) {
		printf("i1->lba_ssd = %llu, i1->lba_hdd = %llu\n", (i1->lba_ssd), (i1->lba_hdd));
		printf("i2->lba_ssd = %d, i2->lba_hdd = %d\n", (int)i2->lba_ssd, (int)i2->lba_hdd);
	}
	del_node(i1);
	i1 = find_item(11);
	if (i1 == NULL) {
		printf("Bazinga! \n");
	}
	
	delete_all();
	i1 = find_item(11);
	i2 = find_item(22);
	if ((i1 == NULL) && (i2 == NULL)) {
		printf("Bazinga²! \n");
	}*/
	
	
	
	
	
	
	
	
	
	
	
}
