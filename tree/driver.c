
#include <asm/uaccess.h>
#include <linux/bio.h>
#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>

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

MODULE_LICENSE("Dual BSD/GPL");



#define MAJOR_SSD 42 /* Major du disque SSD avec lequel on va dialoguer */
#define MINOR_SSD 42 /* Minor du disque SSD avec lequel on va dialoguer */

#define MAJOR_HDD 8 /* Major du disque HDD avec lequel on va dialoguer */
#define MINOR_HDD 0 /* Minor du disque HDD avec lequel on va dialoguer */

static int major_num = 0; /* Numéro major désignant le driver. 0 -> Attribution automatique par le noyau. */

/* Structure du périphérique à créer. */
struct sbd_device {
    struct request_queue *queue; /* File de requêtes */
	struct gendisk *gd;	/* Cette structure permettra au noyau d'obtenir d'avantages d'informations sur le périphérique à créer */
    struct block_device *target_dev; /* Disque dur avec lequel nous souhaitons communiquer (HDD pour l'instant) */

};

/* Structure de notre pilote */
static struct sbd_device Device;


/* 
 * Fonction permettant de rediriger la requête en remplaçant le pilote qui 
 * traitera la bio.
 * Cette fonction est appellée implicitement par le noyau
 */
static int sbull_make_request(struct request_queue *q, struct bio *bio){
    bio->bi_bdev = Device.target_dev; /* Remplacement du pilote */
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


    dev->queue = blk_alloc_queue(GFP_KERNEL); /* Création de la file de requête  */
    if (dev->queue == NULL) /* Vérification du succès de la création de la file */
        return -1; 

    /* Remplace la fonction de traitement des requêtes de la file */
    blk_queue_make_request(dev->queue, sbull_make_request);

    dev->gd = alloc_disk(1); /* Crée la structure et définit le nombre max de minor géré */
    if (!dev->gd){ /* Test pour la création de la structure */
        return -1;
    }

    dev->gd->major = major_num; /* Spécification du numéro major */
    dev->gd->first_minor = 0; /* Spécification du premier numéro minor. Définit le numéro du disque */
    dev->gd->fops = &sbd_ops; /* Spécifie les opérations gérées par le périphérique (ioctl, open, release, read,  etc...) */
    dev->gd->private_data = dev; /* Pointe vers la structure de notre périphérique. Seul le pilote y aura accès */
    dev->gd->queue = dev->queue; /* La file du gendisk pointe vers celle que nous avons créée */
    dev->gd->flags |= GENHD_FL_EXT_DEVT; /* Permet des disques étendues */
    strcpy(dev->gd->disk_name, "pbv"); /* Spécifie le nom du disque créé */

    /* Accéde à la struct du périphérique par le biais d'un couple major/minor */
    dev->target_dev = bdget(MKDEV(MAJOR_HDD,MINOR_HDD));
    /* Test si l'ouverture à été effectuée */
    if (!dev->target_dev || !dev->target_dev->bd_disk)
        return -1;

    /* Récupération de la file de requête du HDD */
    q = bdev_get_queue(dev->target_dev);
    /* Test si erreur */
    if(!q)
        return -1;

    /* 
     * Mise en place des informations de la file de requêtes du gendisk 
     * Elle devra théoriquement cloner celle du HDD
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
    set_capacity(dev->gd, get_capacity(dev->target_dev->bd_disk));

    /* Ajoute le gd aux disques actifs. Il pourra être manipulé par le système */
    add_disk(dev->gd);

    return 0;
}


/*
 * Cette fonction d'initialisation met d'abord en place le périphérique
 * puis initialise une file de requête vide (en attente). Pour finir, elle
 * crée une structure gendisk afin de pouvoir spécifier des informations
 * relatives aux disques durs.
 */
static int __init sbd_init(void) {

    /* Enregistrement auprès du noyau */
    major_num = register_blkdev(0, "pbv"); /* Enregistrement du nouveau disque auprès du noyau, grâce à un numéro major et un nom*/
    if (major_num <= 0) { /* Si une erreur s'est produite (pas de major number attribué) */
        printk(KERN_WARNING "pbv: unable to set major number\n"); /* Inscription dans syslog de l'incapacité du noyau à trouver un major  */
        return -EBUSY;
    }

    /* Configure notre pilote et test le retour de notre fonction */
    if (setup_device(&Device) < 0) {
        unregister_blkdev(major_num, "pbv");
        return -ENOMEM;
    }

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

    if (Device.queue){
        blk_cleanup_queue(Device.queue); /* Vide et désactive la file de requêtes */
    }

    blkdev_put(Device.target_dev, FMODE_READ|FMODE_WRITE|FMODE_EXCL);

    unregister_blkdev(major_num, "pbv"); /* Désactive l'enregistrement auprès du kernel */
}

module_init(sbd_init); /* Initialisation du module */
module_exit(sbd_exit); /* Désactivation du module */

