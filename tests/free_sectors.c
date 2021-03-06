#include "free_sectors.h"

free_sector mem_SSD= { 50, 50, 50, 50, 50, 50, 50, 50}; 

void init_free_sector(uint64_t size, uint64_t occup_max, uint64_t occup_min, uint64_t first_line, uint64_t last_usable) {
	mem_SSD.size_line = size;
	mem_SSD.cpt = 0;
	mem_SSD.occup_max = occup_max;
	mem_SSD.occup_min = occup_min;
	mem_SSD.first_to_delete = first_line;
	mem_SSD.first_to_use = first_line;
	mem_SSD.first_usable = first_line;
	mem_SSD.last_usable = last_usable;
}

/*retourne le premier secteur de la première ligne allouable*/
uint64_t get_line() {
	mem_SSD.cpt++;
	uint64_t res = mem_SSD.first_to_use;
	if (mem_SSD.first_to_use != mem_SSD.last_usable){
		mem_SSD.first_to_use += mem_SSD.size_line;
	} else {
		mem_SSD.last_usable = mem_SSD.first_usable;
	}
	return res;
}

/*supprime les mem_SSD.size premiers blocs pointés par mem_SSD.first_to_delete*/
void free_one_line() {
	mem_SSD.cpt--;
	if (mem_SSD.first_to_delete != mem_SSD.last_usable)
		mem_SSD.first_to_delete += mem_SSD.size_line;
	else 
		mem_SSD.first_to_delete = mem_SSD.first_usable;
}
