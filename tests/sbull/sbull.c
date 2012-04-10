/*
 * A sample, extra-simple block driver. Updated for kernel 2.6.31.
 *
 * (C) 2003 Eklektix, Inc.
 * (C) 2010 Pat Patterson <pat at superpat dot com>
 * Redistributable under the terms of the GNU GPL.
 */

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

MODULE_LICENSE("Dual BSD/GPL");
static char *Version = "1.4";


static int major_num = 0; /* Numéro major désignant le driver. 0 -> Attribution automatique par le noyau. */
module_param(major_num, int, 0); /* Le major number est passé en paramètre au module */
static int logical_block_size = 512; /* Taille de bloc logique */
module_param(logical_block_size, int, 0); /* Cette taille est donnée en paramètre au module */
static int nsectors = 1024; /* Nombre secteur */
module_param(nsectors, int, 0); /* Nombre de secteurs passé en paramètre au module */


#define KERNEL_SECTOR_SIZE 512 /* Définit la taille secteur du noyau. */


static struct request_queue *Queue; /* File de requêtes */

/* Structure du périphérique à créer. */
struct sbd_device {
	unsigned long size;	/* Taille du périphérique en secteurs */
	spinlock_t lock;	/* Spinlock */
	u8 *data1;			/* Le tableau de données 1 */
	u8 *data2;			/* Le tableau de données 2 */
	struct gendisk *gd;	/* objet gendisk. Cette structure permettra au noyau d'obtenir d'avantages d'informations sur le périphérique à créer */
};

static struct sbd_device Device;



/* Traitement d'une requête I/O. */
/* Cette fonction utilise un disque donné (*dev) à un emplacement donné 
 * (sector) sur un nb d'octet donné (nsect). Si "write" est à 1, le
 * traitement sera effectué à partir de la variable buffer. Sinon le
 * résultat sera stocké dans cette dernière. Il s'agit au final de permettre
 * les opérations d'écriture et de lecture.
 */
 int cpt=0;


static void sbd_transfer(struct sbd_device *dev, sector_t sector, unsigned long nsect, char *buffer, int write) {
	unsigned long offset = sector * logical_block_size; /* Calcul de l'adresse cible */
	unsigned long nbytes = nsect * logical_block_size; /* Calcul le nombre d'octets */

	if ((offset + nbytes) > dev->size) { /* Dans le cas, ou la quantité de bytes à écrire est trop importante par rapport à la taille du périphérique */
		printk (KERN_NOTICE "sbd: Beyond-end write (%ld %ld)\n", offset, nbytes); /* Inscription dans syslog d'une écriture hors limite */
		return;
	}
	if (write) /* S'il s'agit d'une requête d'écriture */
	{
		memcpy(dev->data1 + offset, buffer, nbytes); /* Ecriture à l'offset demandé (data+offset) de nbytes octets (Opération d'écriture)*/
		memcpy(dev->data2 + offset, buffer, nbytes); /* Ecriture à l'offset demandé (data+offset) de nbytes octets (Opération d'écriture)*/
	}
	else
	{
		if (cpt == 0 )
		{
			memcpy(buffer, dev->data1 + offset, nbytes); /* Copie de data+offset dans buffer sur nbytes octets (Opération de lecture) */
			printk(KERN_INFO "Lu a partir de segment 1");
			cpt = 1;
		}
		else
		{
			memcpy(buffer, dev->data2 + offset, nbytes); /* Copie de data+offset dans buffer sur nbytes octets (Opération de lecture) */
			printk(KERN_INFO "Lu a partir de segment 2");
			cpt = 0;
		}
	}
}

/* 
 * Cette fonction permet de sélectionner une requête dans une file
 * donnée (q) et de l'envoyer à la fonction sbd_transfert afin de la
 * traiter.
 * Une requête peut être composée de plusieurs "morceaux". Dans cette 
 * fonction, chaque "morceau" de la requête sera traité consécutivement
 * jusqu'à ce que cette dernière soit traitée entièrement. 
 */
static void sbd_request(struct request_queue *q) {
	struct request *req; /* Instancie la requête */

	req = blk_fetch_request(q); /* Sélection de la requête dans la file */
	while (req != NULL) { /* Tant que la requête n'est pas nulle, i.e. file de requête n'est pas vide */
		if (req == NULL || (req->cmd_type != REQ_TYPE_FS)) { /* Si requête nulle ou n'ayant pas le type "fs", i.e. s'il ne s'agit pas d'une requête liée au système de fichiers */
			printk (KERN_NOTICE "Skip non-CMD request\n"); /* Inscription dans syslog de la non-exécution de la requête */
			__blk_end_request_all(req, -EIO); /* Finition de la requête */
			continue; /* Ignore les instructions suivantes et effectue un nouveau tour de boucle */
		}
		sbd_transfer(&Device, blk_rq_pos(req), blk_rq_cur_sectors(req),
				req->buffer, rq_data_dir(req)); /* Traitement de la requete */
		if ( ! __blk_end_request_cur(req, 0) ) { /* Si la requete n'est pas complètement traitée */
			req = blk_fetch_request(q); /* Sélectionne la suite de la requête dans la file */
		}
	}
}

static int sbull_xfer_bio(struct sbd_device* dev, struct bio* bio){
    int i;
    struct bio_vec *bvec;
    sector_t sector = bio->bi_sector;

    /* Do each segment independently */
    bio_for_each_segment(bvec, bio, i){
        char* buffer = __bio_kmap_atomic(bio, i , KM_USER0);
        sbd_transfer(dev, sector, (bio_cur_bytes(bio)>>9),
                        buffer, bio_data_dir(bio) == WRITE);
        sector += bio_cur_bytes(bio)>>9;
        __bio_kunmap_atomic(bio, KM_USER0);
    }
    return 0; /* Always "succeed" */
}

static int sbull_make_request(struct request_queue *q, struct bio *bio){
    int status;
    struct sbd_device *dev = q->queuedata;
    /* Obtention de l'id du driver du HDD */
/*    struct block_device* _bi_dev = vmalloc(sizeof(struct block_device));
    _bi_dev->bd_dev = MKDEV(8,0);
    bio->bi_bdev = _bi_dev;*/
    bio->bi_bdev = open_by_devnum(MKDEV(8,0), FMODE_READ|FMODE_WRITE);
    /* Transfère la requête au device en question */ 
/*    status = sbull_xfer_bio(dev, bio); */
    bio_endio(bio, 0);
    return 1;
} 




/*
 * Depuis que blkdev_ioctl appelle getgeo au lieu de ioctl, nous devons
 * redéfinir une fonction sbd_getgeo qui spécifie les informations à propos 
 * d'un disque dur physique et qui puisse définir un emplacement.
 */
int sbd_getgeo(struct block_device * block_device, struct hd_geometry * geo) {
	long size;

	/* We have no real geometry, of course, so make something up. */
	size = Device.size * (logical_block_size / KERNEL_SECTOR_SIZE);
	geo->cylinders = (size & ~0x3f) >> 6;
	geo->heads = 4;
	geo->sectors = 16;
	geo->start = 0;
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
 * Cette fonction d'initialisation met d'abord en place le périphérique
 * puis initialise une file de requête vide (en attente). Pour finir,
 * elle crée une structure gendisk afin de pouvoir spécifier des informations
 * relatives aux disques durs.
 */
static int __init sbd_init(void) {
	
	/* Mise en place du phériphérique interne */
	Device.size = nsectors * logical_block_size; /* Taille de notre périphérique */
	spin_lock_init(&Device.lock); /* Initialisation du spinlock */
	Device.data1 = vmalloc(Device.size); /* Alloue la zone mémoire n°1 de la taille du périphérique */
	if (Device.data1 == NULL) /* Teste si le tableau de données n°1 est vide */
		return -ENOMEM; /* Retourne l'erreur "Out of Memory" */
	Device.data2 = vmalloc(Device.size); /* Alloue la zone mémoire n°2 de la taille du périphérique */
	if (Device.data2 == NULL) /* Teste si le tableau de données n°2 est vide */
		return -ENOMEM; /* Retourne l'erreur "Out of Memory" */


	/* Mise en place de la file de requête */
	//Queue = blk_init_queue(sbd_request, &Device.lock); /* Initialise une file de requête */
    Queue = blk_alloc_queue(GFP_KERNEL);
	if (Queue == NULL) /* Vérification du succès de la création de la file */
		goto out; /* Saut à l'instruction out */
    
    /* Utile seulement si on ne veut pas utiliser de file de requêtes*/
    blk_queue_make_request(Queue, sbull_make_request);
    

	blk_queue_logical_block_size(Queue, logical_block_size); /* Spécifie la taille des blocs (en octets) de la file  */

	/* Enregistrement auprès du noyau */
	major_num = register_blkdev(major_num, "sbd"); /* Enregistrement du nouveau périphérique auprès du noyau, à partir de son numéro major et d'un nom de device*/
	if (major_num <= 0) { /* Si une erreur s'est produite (pas de major number attribué) */
		printk(KERN_WARNING "sbd: unable to get major number\n"); /* Inscription dans syslog l'avertissement de l'incapacité du noyau à trouver un numéro major  */
		goto out; /* Saute à l'instruction out */
	}
	
	/* Mise en place de la structure GENDISK (gd) */
	Device.gd = alloc_disk(16); /* Crée la structure et définit le nombre max de minor géré */
	if (!Device.gd) /* Teste si la structure n'a pas pu être créée*/
		goto out_unregister;
	Device.gd->major = major_num; /* Spécification du numéro major */
	Device.gd->first_minor = 0; /* Spécification du premier numéro minor. Il permet de connaître le nom du disque à manipuler, si des partitions sont présentes. */
	Device.gd->fops = &sbd_ops; /* Spécifie les opérations gérées par le périphérique (ioctl, open, release, etc...) */
	Device.gd->private_data = &Device; /* Pointe vers la structure spécifique de notre périphérique. Seul le driver y aura accès */
	strcpy(Device.gd->disk_name, "sbd0"); /* Spécifie le nom du disque du gd créé */
	set_capacity(Device.gd, nsectors); /* Spécifie la taille du disque en secteurs (%512 octets) */
	Device.gd->queue = Queue; /* La file du gd pointe vers celle que nous avons créée */
	add_disk(Device.gd); /* Ajoute le gd aux disques actifs. Il pourra être manipulé par le système. */

	return 0;

out_unregister:
	unregister_blkdev(major_num, "sbd"); /* Désactive l'enregistrement auprès du kernel */
out:
	vfree(Device.data1); /* Libère l'espace mémoire n°1 utilisé par le périphérique */
	vfree(Device.data2); /* Libère l'espace mémoire n°2 utilisé par le périphérique */
	return -ENOMEM; /* Retourne l'erreur "Out of Memory" */
}


/*
 * Cette fonction permet la libération de l'espace disque et la 
 * désinscription du module du kernel.
 */
static void __exit sbd_exit(void)
{
	del_gendisk(Device.gd); /* Supprime toutes les informations associées à la structure gendisk (et donc, au disque). Après cette action, le périphérique ne pourra 
				    plus recevoir d'instructions*/
	put_disk(Device.gd); /* Il reste cependant la référence dee notre structure dans le noyau. Il faut donc la supprimer. */
	unregister_blkdev(major_num, "sbd"); /* Désactive l'enregistrement auprès du kernel */
	blk_cleanup_queue(Queue); /* Vide et désactive la file de requêtes */
	vfree(Device.data1); /* Libère l'espace mémoire utilisée pour la structure*/
	vfree(Device.data2); /* Libère l'espace mémoire utilisée pour la structure*/
}

module_init(sbd_init); /* Initialisation du module */
module_exit(sbd_exit); /* Désactivation du module */

