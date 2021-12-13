#!/bin/bash

source device_sd.txt


rm -rf sd_dumped.img sd_burned.img sd_burned.zip dyn_file.h opti.h 
cmd /c upx.exe stage1.exe 
cmd /c upx.exe stage2.exe 
unzip sd_dumped.zip 
echo -ne "\r\n" | ./sdtool.exe sd_dumped.img stage1.exe stage2.exe dyn_file.h opti.h dreglaptop
mv sd_dumped.img sd_burned.img 
zip $(date '+sd_burned_%Y-%m-%d--%H-%M-%S').zip sd_burned.img stage1.exe stage2.exe dyn_file.h opti.h
rm -rf ../at90/evilmass/dyn_file.h 
rm -rf ../at90/evilmass/opti.h
mv dyn_file.h ../at90/evilmass/
mv opti.h  ../at90/evilmass/
./burn.sh sd_burned.img 