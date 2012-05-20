typedef struct free_sector {
	int size_line; /*taille des lignes écrites sur le SSD (en nombre de secteurs)*/
	int cpt; /*nombre de lignes occupées sur le SSD*/
	int occup_max; /*nombres maximal de lignes à allouer sur le SSD avant de déclencher la procédure de vidage du SSD*/
	int occup_min; /*nombre de lignes en dessous duquel on peut arrêter la procédure de vidage*/
	int first_to_delete; /*lba du SSD du premier secteur de la première ligne à supprimer*/
	int first_to_use; /*lba du SSD du premier secteur de la première ligne que l'on peut allouer*/
	int first_usable; /*lba du premier sector à disposition du mapping sur le SSD*/
	int last_usable; /*lba du dernier sector à disposition du mapping sur le SSD*/
}

int get_line();
void free_one_line();
void init_free_sector(int size, int occup_max, int occup_min, int first_line, int last_usable);
