#include "free_sectors.h"

struct *free_sector mem_SSD;

void init_free_sector(int size, int occup_max, int occup_min, int first_line, int last_usable) {
	mem_SSD = (free_sector*) malloc(sizeof(free_sector));
	mem_SSD->size_line = size;
	mem_SSD->cpt = 0;
	mem_SSD->occup_max = occup_max;
	mem_SSD->occup_min = occup_min;
	mem_SSD->first_to_delete = first_line;
	mem_SSD->first_to_use = first_line;
	mem_SSD->first_usable = first_line;
	mem_SSD->last_usable = last_usable;
}

/*retourne le premier secteur de la première ligne allouable*/
int get_line() {
	mem_SSD->cpt++;
	int res = mem_SSD->first_to_use;
	if (mem_SSD->first_to_use != mem_SSD->last_usable)
		mem_SSD->first_to_use += mem_SSD->size_line;
	else 
		mem_SSD->last_usable = mem_SSD->first_usable
	return res;
}

/*supprime les mem_SSD->size premiers blocs pointés par mem_SSD->first_to_delete*/
void free_one_line() {
	mem_SSD->cpt--;
	if (mem_SSD->first_to_delete != mem_SSD->last_usable)
		mem_SSD->first_to_delete += mem_SSD->size_line;
	else 
		mem_SSD->last_delete = mem_SSD->first_usable;
}
