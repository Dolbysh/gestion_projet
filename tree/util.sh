#! /bin/bash

# Configuration
# A modifier si besoin
PATH_HDD="/dev/loop0"
PATH_SSD="/dev/loop1"

if [ ! -e $PATH_HDD ]; then
    echo "$PATH_HDD does not exist"
    exit 1
fi

if [ ! -e $PATH_SSD ]; then
    echo "$PATH_SSD does not exist"
    exit 1
fi

if [ $# -ne 1 ]; then
    echo "Usage : $0 {insert|remove}"
    exit 1
fi

if [ $1 == "insert" ]; then 
    major_hdd=$(ls -l $PATH_HDD|cut -d" " -f 5|cut -d"," -f 1)
    minor_hdd=$(ls -l $PATH_HDD|cut -d" " -f 6)
    major_ssd=$(ls -l $PATH_SSD|cut -d" " -f 5|cut -d"," -f 1)
    minor_ssd=$(ls -l $PATH_SSD|cut -d" " -f 6)

    insmod driver_fonctionnel.ko MAJOR_HDD=major_hdd MINOR_HDD=minor_hdd MAJOR_SSD=major_ssd MINOR_SSD=minor_ssd

elif [ $1 == "remove" ]; then
    rmmod driver_fonctionnel
else 
    echo "Wrong parameter"
    exit 1
fi
