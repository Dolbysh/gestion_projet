obj-m += driver_pbv.o
driver_pbv-objs += free_sectors.o driver.o mapping.o


KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
loop0_attached := $(shell losetup -a|grep /dev/loop0|wc -l)
loop1_attached := $(shell losetup -a|grep /dev/loop1|wc -l)


default:
	dd if=/dev/zero of=test_hdd bs=512 count=200000 && dd if=/dev/zero of=test_ssd bs=512 count=20048
	losetup /dev/loop0 test_hdd && losetup /dev/loop1 test_ssd
	$(MAKE) -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	$(MAKE) -I/usr/include -C $(KDIR) SUBDIRS=$(PWD) clean
	rm -f *.o *.mod.* *.ko.* test_ssd test_hdd
	$(shell losetup -d /dev/loop0 2>/dev/null)
	$(shell losetup -d /dev/loop1 2>/dev/null)
