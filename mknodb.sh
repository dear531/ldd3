#!/bin/bash
#scull read and write for block
module="scull_block"
device="scullb"
mode="644"

rm -rf  /dev/${device}[0-1]

/sbin/insmod ./$module.ko $* || exit 1
major=$(awk '{if ($2 == "scullb") print $1}' /proc/devices)

mknod /dev/${device}0 c $major 0

group="staff"
grep -q '^staff:' /etc/group || group="wheel"
chgrp $group /dev/${device}[0-1]
chmod $mode /dev/${device}[0-1]
