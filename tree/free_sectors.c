#include "free_sectors.h"

struct *free_sector mem_SSD;

void init_free_sector(int size, int occup_max, int occup_min, int first_to_delete, int first_to_use, int first_usable, int last_usable) {
	mem_SSD = (free_sector*) malloc(sizeof(free_sector));
	mem_SSD->size_line = size;
	mem_SSD->cpt = 0;
	mem_SSD->occup_max = occup_max;
	mem_SSD->occup_min = occup_min;
	mem_SSD->first_to_delete = first_to_delete;
	mem_SSD->first_to_use = first_to_use;
	mem_SSD->first_usable = first_usable;
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

/*
typedef struct free_sector {
	int size_line; /*taille des lignes écrites sur le SSD (en nombre de secteurs)
	int cpt; /*nombre de lignes occupées sur le SSD
	int occup_max; /*nombres maximal de lignes à allouer sur le SSD avant de déclencher la procédure de vidage du SSD
	int occup_min; /*nombre de lignes en dessous duquel on peut arrêter la procédure de vidage
	int first_to_delete; /*lba du SSD du premier secteur de la première ligne à supprimer
	int first_to_use; /*lba du SSD du premier secteur de la première ligne que l'on peut allouer
	int first_usable; /*lba du premier sector à disposition du mapping sur le SSD
	int last_usable; /*lba du dernier sector à disposition du mapping sur le SSD
}

int get_line();
void free_line();
void init_free_sector(int size, int occup_max, int occup_min, int first_to_delete, int first_to_use, int first_usable, int last_usable);
*/
