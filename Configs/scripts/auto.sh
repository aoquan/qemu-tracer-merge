#!/bin/sh

#./insertintodb.rb $4 将符号表导入数据库中，同时将文件导入磁盘镜像中。由另一个按钮实现
#之前还需实现读取数据库中的信息，将内核两个边界、busybox两个边界、modules地址存入文件中

#将脚本文件导入自动启动中

MAIN=$(pwd)
BUSYBOX=$MAIN/$2-busybox

cp rcS $BUSYBOX/_install/etc/init.d/
cd $BUSYBOX/_install
cat $3>>etc/init.d/rcS
find . |cpio -o --format=newc > $MAIN/$2-rootfs.img

#自动测试

cd $MAIN

./qemu-system-$2 -kernel $1/$2/bzImage -initrd $2-rootfs.img -append "root=/dev/ram rdinit=sbin/init" -d in_asm -D log1

./parse.rb testcase testresult
