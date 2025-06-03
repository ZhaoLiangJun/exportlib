#!/bin/sh 

#ldd /usr/bin/perf  |cut -d ">" -f 2|grep lib|cut -d "(" -f 1|xargs tar -chvf 123.tar
  
exe="./Xorg" #发布的程序名称  
des="/root/tmp/Export-lib/lib" #打包程序路径 

rm -rf $des
mkdir $des 
  
deplist=$(ldd $exe | awk  '{if (match($3,"/")){ printf("%s "),$3 } }') 

echo $deplist

cp $deplist $des 
