obj-m := driver_fonctionnel.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)
loop0_attached := $(shell losetup -a|grep /dev/loop0|wc -l)
loop1_attached := $(shell losetup -a|grep /dev/loop1|wc -l)


default:
	dd if=/dev/zero of=test_hdd bs=512 count=200000 && dd if=/dev/zero of=test_ssd bs=512 count=20048
	mke2fs -F test_hdd && mke2fs -F test_ssd
	losetup /dev/loop0 test_ssd && losetup /dev/loop1 test_hdd
	$(MAKE) -I/usr/include -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	$(MAKE) -I/usr/include -C $(KDIR) SUBDIRS=$(PWD) clean
	rm -f *.mod *.ko test_ssd test_hdd

    ifeq ($(loop0_attached), 1)
      $(shell losetup -d /dev/loop0)
    endif

    ifeq ($(loop1_attached), 1)
      $(shell losetup -d /dev/loop1)
    endif
