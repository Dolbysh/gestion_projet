
#include <asm/uaccess.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <pthread.h>
#include <math.h>

#include <linux/kernel.h> /* printk() */
#include <linux/fs.h>     /* everything... */
#include <linux/errno.h>  /* error codes */
#include <linux/types.h>  /* size_t */
#include <linux/vmalloc.h>
#include <linux/genhd.h>
#include <linux/blkdev.h>
#include <linux/hdreg.h>
#include <linux/kdev_t.h>
#include <linux/timer.h>
#include "mapping.h"
#include "free_sectors.c"

#include 
MODULE_LICENSE("Dual BSD/GPL");


#define LINE_SIZE 1 /* Taille d'une ligne */
#define TAUX_MIN 70 /* Taux d'occupation au dessous duquel 
on doit revenir lors d'une procédure de vidage du SSD */
#define TAUX_MAX 90 /* Taux d'occupation maximal du SSD */

#define MAJOR_SSD 7 /* Major du disque SSD avec lequel on va dialoguer */
#define MINOR_SSD 0 /* Minor du disque SSD avec lequel on va dialoguer */

#define MAJOR_HDD 7 /* Major du disque HDD avec lequel on va dialoguer */
#define MINOR_HDD 1 /* Minor du disque HDD avec lequel on va dialoguer */


/*Mode de fonctionnement*/

#define SECURITE 0 /* On écrit sur les deux disques systématiquement */ 
#define ECONOMIE 1 /* On écrit uniquement sur les SSD */

static int mode = SECURITE;

static int major_num = 0; /* Numéro major désignant le driver. 0 -> Attribution automatique par le noyau. */

/* Structure du périphérique à créer. */
struct sbd_device {
    struct request_queue *queue; /* File de requêtes */
	struct gendisk *gd;	/* Cette structure permettra au noyau d'obtenir d'avantages d'informations sur le périphérique à créer */
    struct block_device *target_hdd; /* Disque dur avec lequel nous souhaitons communiquer (HDD pour l'instant) */
	struct block_device *target_ssd;
};

/* Structure de notre pilote */
static struct sbd_device Device;

/* fonctions d'aide */
/* Premier secteur de la dernière ligne utilisable sur le SSD */
u64 compute_last_usable(u64 i){
	return (u64) floor(i / LINE_SIZE) * LINE_SIZE - LINE_SIZE + 1 ;
}

/* Nombre maximal de lignes pouvant être utilisées sur le SSD */
u64 compute_max(u64 i){
	return (u64) floor(i * TAUX_MAX / (LINE_SIZE * 100));
}

/* Nombre de lignes utilisées sur le SSD en dessous duquel il faut arriver après l'avoir vidé */
u64 compute_min(u64 i){
	return (u64) floor(i * TAUX_MIN / (LINE_SIZE * 100));
}

void ssd_transfer(int sector, struct bio *bio){
	bio->bi_bdev = Device.target_ssd;
	bio->bi_sector = sector;
	generic_make_request(clone);
}

/* Fonction appelée lorsque le SSD est trop rempli => transfert d'une partie du SSD vers le HDD */
void ssd_empty(){
	node* n = mapping;
	node* temp = NULL;
	int nb_to_delete = mem_SSD.occup_max - mem_SSD.occup_min;
	for (i = 0; i < nb_to_delete; i++) {
		temp = n->hh.next;
		del_node(n);
		n = temp;
		if (n == NULL) break;
		////rajouter code pour le mode économie
		free_one_line();
		/*décaler les pointeurs de mem_SSD */
	}
}		

/* 
 * Fonction permettant de rediriger la requête en remplaçant le pilote qui 
 * traitera la bio.
 * Cette fonction est appellée implicitement par le noyau
 */
static int passthrough_make_request(struct request_queue *q, struct bio *bio)
{
    int request_type = bio_data_dir(bio);
    sector_t offset = bio->bi_sector;
    node* n = find_item(offset);
    if (request_type == READ){
        printk(KERN_WARNING "Make request : READ \n");
		int offset = bio->bi_sector;
		
		if (n == NULL){
			bio->bi_bdev = Device.target_hdd;
			struct bio *clone = bio_clone(bio, GFP_KERNEL);
			sector = alloc_a_line();
			clone->bi_rw = WRITE;
		        if (pthread_create(&thread, NULL, ssd_transfer, sector, clone)) {
   				printk(KERN_WARNING "pthread_create failed");
			}
			add_node(offset, sector);
		} else {
			bio->bi_bdev = Device.target_ssd;
			bio->bi_sector = n->lba_ssd;	
		}
    } else if (request_type == WRITE){
	switch (mode) {
		case SECURITE:
			printk(KERN_WARNING "Make request : WRITE BEGIN \n");
			struct bio *clone = bio_clone(bio,GFP_KERNEL);
			bio->bi_bdev = Device.target_hdd;
			if (n == NULL) {
				sector = alloc_a_line();
				if (pthread_create(&thread, NULL, ssd_transfer, sector, clone)) {
   					printk(KERN_WARNING "pthread_create failed");
				}
				add_node(offset, sector);
			} else {
				if (pthread_create(&thread, NULL, ssd_transfer, sector, clone)) {
   					printk(KERN_WARNING "pthread_create failed");
				}	
				printk(KERN_WARNING "Make request : WRITE END \n");
			}
			break;
		case ECONOMIE:
			sector = get_line();
			if (pthread_create(&thread, NULL, ssd_transfer, sector, bio)) {
   				printk(KERN_WARNING "pthread_create failed");
			}
			add_node(offset,sector);
			break;	
		default:
			printk(KERN_WARNING "Make request : Mode inattendu \n");
			return -1;
	}
    }

    if(HASH_COUNT(mapping) >= mem_ssd->occupy_max){
	ssd_empty();
	}
	
    return 1;
}

/* 
 * Fonction spécifiant les informations à propos de la géométrie du disque
 * dur (virtuel ou non) pour les utiliser (e.g.  fdisk).
 * Apparement inutile mais doit etre implémentée.
 */
int sbd_getgeo(struct block_device * block_device, struct hd_geometry * geo) {

    geo->heads = 25;
    geo->sectors = 3;

    geo->cylinders = get_capacity(block_device->bd_disk);
    sector_div(geo->cylinders, geo->heads * geo->sectors);

    return 0;
}


/*
 * Structure contenue dans blkdev qui spécifie les opérations gérées
 * par notre périphérique bloc.
 */
static struct block_device_operations sbd_ops = {
    .owner  = THIS_MODULE,
    .getgeo = sbd_getgeo
};


/*
 * Fonction qui configure de la structure de notre pilote.
 * Gendisk, file de requete, disque cible.
 */
static int setup_device (struct sbd_device* dev){
    struct request_queue *q;

    dev->target_hdd = vmalloc(sizeof(struct block_device));

    if (dev->target_hdd == NULL){
        printk(KERN_WARNING "----------dev");
        return -1;
    }

    dev->target_ssd = vmalloc(sizeof(struct block_device));

    if (dev->target_ssd == NULL){
        printk(KERN_WARNING "----------ssd");
        return -1;
    }

    dev->queue = blk_alloc_queue(GFP_KERNEL); /* Création de la file de requête  */
    if (dev->queue == NULL){ /* Vérification du succès de la création de la file */
        printk(KERN_WARNING "blk_alloc_queue FAILED");
        return -1; 
    }
    printk(KERN_WARNING "blk_alloc_queue DONE");

    /* Remplace la fonction de traitement des requêtes de la file */
    blk_queue_make_request(dev->queue, passthrough_make_request);
    printk(KERN_WARNING "blk_queue_make_request DONE");

    dev->gd = alloc_disk(1); /* Crée la structure et définit le nombre max de minor géré */
    if (!dev->gd){ /* Test pour la création de la structure */
    printk(KERN_WARNING "alloc_disk FAILED");
        return -1;
    }

    printk(KERN_WARNING "alloc_disk DONE");
    dev->gd->major = major_num; /* Spécification du numéro major */
    dev->gd->first_minor = 0; /* Spécification du premier numéro minor. Définit le numéro du disque */
    dev->gd->fops = &sbd_ops; /* Spécifie les opérations gérées par le périphérique (ioctl, open, release, read,  etc...) */
    dev->gd->private_data = dev; /* Pointe vers la structure de notre périphérique. Seul le pilote y aura accès */
    dev->gd->queue = dev->queue; /* La file du gendisk pointe vers celle que nous avons créée */
    dev->gd->flags |= GENHD_FL_EXT_DEVT; /* Permet des disques étendues */
    strcpy(dev->gd->disk_name, "pbv"); /* Spécifie le nom du disque créé */

    /* Accéde à la struct du périphérique par le biais d'un couple major/minor */
    dev->target_hdd = open_by_devnum(MKDEV(MAJOR_HDD,MINOR_HDD), FMODE_READ|FMODE_WRITE|FMODE_EXCL); /* bdget(MKDEV(MAJOR_HDD,MINOR_HDD)); */
    /* Test si l'ouverture à été effectuée */
    if (!dev->target_hdd || !dev->target_hdd->bd_disk){
        printk(KERN_WARNING "bdget hdd FAILED");
        return -1;
    }
    printk(KERN_WARNING "bdget hdd DONE");

    dev->target_ssd = open_by_devnum(MKDEV(MAJOR_SSD,MINOR_SSD), FMODE_READ|FMODE_WRITE|FMODE_EXCL); /*bdget(MKDEV(8,16));*/

    if (!dev->target_ssd || !dev->target_ssd->bd_disk){
    printk(KERN_WARNING "bdget ssd FAILED");
        return -1;
    }
    printk(KERN_WARNING "bdget ssd DONE");

    /* Récupération de la file de requête du HDD et du SSD */
    q1 = bdev_get_queue(dev->target_hdd);	
    q = bdev_get_queue(dev->target_ssd);
    /* Test si erreur */
    if(!q){
        printk(KERN_WARNING "bdev_get_queue FAILED");
        return -1;
    }
    printk(KERN_WARNING "bdev_get_queue DONE");

    if (q->limits.max_sectors > q1->limits.max_sectors){
	 printk(KERN_WARNING "setup_device: SSD de taille supérieure au HDD");	
	 return -1;
	}
    /* 
     * Mise en place des informations de la file de requêtes du gendisk 
     * Elle devra théoriquement cloner celle du SSD
     */
    dev->gd->queue->limits.max_hw_sectors      = q->limits.max_hw_sectors;
    dev->gd->queue->limits.max_sectors         = q->limits.max_sectors;
    dev->gd->queue->limits.max_segment_size    = q->limits.max_segment_size;
    dev->gd->queue->limits.logical_block_size  = 512;
    dev->gd->queue->limits.physical_block_size = 512;
    //set_bit(QUEUE_FLAG_NONROT, &dev->gd->queue->queue_flags); /* Pour le SSD */
    /* non-rotational device (SSD)
     * Explication trouvée :  
     * "drivers for solid-state devices can set QUEUE_FLAG_NONROT to hint 
     * that seek time optimizations may be sub-optimal" 
     * */

    /* Fixe la taille du périphérique à celle du périphérique HDD */
    set_capacity(dev->gd, get_capacity(dev->target_ssd->bd_disk));

    /* Ajoute le gd aux disques actifs. Il pourra être manipulé par le système */
    add_disk(dev->gd);

    /*On initialise notre structure free_sectors */
    u64 disk_size = q1->limits.max_sectors;
    	
    void init_free_sector(1, occup_max(disk_size), compute_min(disk_size), 11, compute_last_usable(disk_size));
    printk(KERN_WARNING "add_disk DONE");

    return 0;
}


/*
 * Cette fonction d'initialisation met d'abord en place le périphérique
 * puis initialise une file de requête vide (en attente). Pour finir, elle
 * crée une structure gendisk afin de pouvoir spécifier des informations
 * relatives aux disques durs.
 */
static int __init sbd_init(void) {

    printk(KERN_WARNING "BEGIN init");
    /* Enregistrement auprès du noyau */

    major_num = register_blkdev(0, "pbv"); /* Enregistrement du nouveau disque auprès du noyau, grâce à un numéro major et un nom*/
    printk(KERN_WARNING "register_blkdev DONE");

    if (major_num <= 0) { /* Si une erreur s'est produite (pas de major number attribué) */
        printk(KERN_WARNING "pbv: unable to set major number\n"); /* Inscription dans syslog de l'incapacité du noyau à trouver un major  */
        return -EBUSY;
    }

    /* Configure notre pilote et test le retour de notre fonction */
    if (setup_device(&Device) < 0) {
        printk(KERN_WARNING "setup_device FAILED");
        unregister_blkdev(major_num, "pbv");
        return -ENOMEM;
    }

    printk(KERN_WARNING "END init");
    return 0;
}


/*
 * Cette fonction permet la libération de l'espace disque et la 
 * désinscription du module du kernel.
 */
static void __exit sbd_exit (void) {
    if (Device.gd){
        /* Supprime toutes les informations associées à la structure gendisk (et
         * donc, au disque). Après cette action, le périphérique ne pourra plus 
         * recevoir d'instructions*/
        del_gendisk(Device.gd);

        /* Il reste cependant la référence de notre structure dans le noyau.
         * Il faut donc la supprimer. */; 
        put_disk(Device.gd);
    }

    if (Device.queue) 
        blk_cleanup_queue(Device.queue); /* Vide et désactive la file de requêtes */

    if (Device.target_hdd)
        blkdev_put(Device.target_hdd, FMODE_READ|FMODE_WRITE|FMODE_EXCL);

    if (Device.target_ssd)
        blkdev_put(Device.target_ssd, FMODE_READ|FMODE_WRITE|FMODE_EXCL);
    unregister_blkdev(major_num, "pbv"); /* Désactive l'enregistrement auprès du kernel */
}

module_init(sbd_init); /* Initialisation du module */
module_exit(sbd_exit); /* Désactivation du module */

