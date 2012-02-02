#Create two test disks 
#dd if=/dev/zero of=/tmp/disk1 bs=1024 seek=2047 count=1
#dd if=/dev/zero of=/tmp/disk2 bs=1024 seek=2047 count=1

#losetup /dev/loop1 /tmp/disk1
#losetup /dev/loop2 /tmp/disk2

#mke2fs /dev/loop1
#mke2fs /dev/loop2

#Unmount devices to allow creation of the aggregate
#umount /dev/loop1
#umount /dev/loop2

#Load device mapper 
#modprobe dm-mod


#create block devices
echo 0 $(blockdev --getsize /dev/loop1) linear /dev/loop1 0 > table
echo $(blockdev --getsize /dev/loop1) $(blockdev --getsize /dev/loop2) linear /dev/loop2 0 >> table

dmsetup create aggregate_test table

