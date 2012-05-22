#include <linux/types.h>
//#include <stdint.h>


typedef struct FREESECTOR {
       uint64_t size_line; /*taille des lignes écrites sur le SSD (en nombre de secteurs)*/
       uint64_t cpt; /*nombre de lignes occupées sur le SSD*/
       uint64_t occup_max; /*nombres maximal de lignes à allouer sur le SSD avant de déclencher la procédure de vidage du SSD*/
       uint64_t occup_min; /*nombre de lignes en dessous duquel on peut arrêter la procédure de vidage*/
       uint64_t first_to_delete; /*lba du SSD du premier secteur de la première ligne à supprimer*/
       uint64_t first_to_use; /*lba du SSD du premier secteur de la première ligne que l'on peut allouer*/
       uint64_t first_usable; /*lba du premier sector à disposition du mapping sur le SSD*/
       uint64_t last_usable; /*lba du dernier sector à disposition du mapping sur le SSD*/
} free_sector;

uint64_t get_line(void);
void free_one_line(void);
void init_free_sector(uint64_t size, uint64_t occup_max, uint64_t occup_min, uint64_t first_line, uint64_t last_usable);
