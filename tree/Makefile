obj-m := driver_.o

KDIR := /lib/modules/$(shell uname -r)/build
PWD := $(shell pwd)

default:
	dd if=/dev/zero of=test_hdd bs=512 count=20048
    dd if=/dev/zero of=test_ssd bs=512 count=20048
	mount -o loop=/dev/loop0 test_ssd /mnt/ssd
	mount -o loop=/dev/loop1 test_hdd /mnt/hdd
	$(MAKE) -I/usr/include -C $(KDIR) SUBDIRS=$(PWD) modules

clean:
	$(MAKE) -I/usr/include -C $(KDIR) SUBDIRS=$(PWD) clean
	rm -f *.mod *.ko *~
