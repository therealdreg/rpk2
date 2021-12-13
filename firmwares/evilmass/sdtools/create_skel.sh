#!/bin/bash

function doer() {

busy=true
while $busy
do
 if mountpoint -q "$1"
 then
  umount "$1" 2> /dev/null
  if [ $? -eq 0 ]
  then
   busy=false  # umount successful
   echo "umount successful!"
  else
   echo -n '.'  # output to show that the script is alive
   sleep 5      # 5 seconds for testing, modify to 300 seconds later on
  fi
 else
  busy=false  # not mounted
 fi
done
}


set -x 

cd "$(dirname "${BASH_SOURCE[0]}")"

apt-get install dosfstools 
apt-get install zip
apt-get install mtools

doer sdpt
rm -rf sd.img sd.zip
dd if=/dev/zero of=sd.img bs=1M count=10
mkfs -t vfat -f 1 -F 16 -n DREG sd.img
mkdir sdpt
mount -t auto -o loop sd.img sdpt
perl -e 'print "r" x 512' > sdpt/r
perl -e 'print "d" x 6291456' > sdpt/d.exe
df -h sdpt 
doer sdpt
rm -rf sdpt
file sd.img
minfo -i sd.img
zip sd.zip sd.img
rm -rf sd.img