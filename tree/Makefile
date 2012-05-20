obj-m := driver_.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	dd if=/dev/zero of=test_hdd bs=512 count=200000 && dd if=/dev/zero of=test_ssd bs=512 count=20048
	mke2fs -F test_hdd && mke2fs -F test_ssd
	losetup /dev/loop0 test_ssd && losetup /dev/loop1 test_hdd
	$(MAKE) -I/usr/include -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	losetup -d /dev/loop0 && losetup -d /dev/loop1
	$(MAKE) -I/usr/include -C $(KDIR) SUBDIRS=$(PWD) clean
	rm -f *.mod *.ko *~
