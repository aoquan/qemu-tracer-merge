#!/bin/sh
PLAT=/home/crdong/main/x86_32
INSTALL=$PLAT/busybox-1.22.1/_install

cd $PLAT
cp -r ./busybox-1.22.1/install $INSTALL

mv /mnt/freenas/DCG-RTL/testcase/testcase1/* ./test_case
tstcs=`ls  ./test_case`  #get test case name

if [ "${tstcs}" = "" ]
then 
echo "error please put test case"
exit 1
fi

tar -zxf ./test_case/${tstcs} -C ./test_case  #decompress test case
rm -rf ./test_case/*.tar.gz                   #delete .tar.gz

echo ./a.sh >> $INSTALL/etc/init.d/rcS
mv ./test_case/* $INSTALL


cd $INSTALL
echo poweroff>>etc/init.d/rcS
find . | cpio -o --format=newc > $PLAT/rootfs.img

cd $PLAT
./qemu -kernel linux-3.5.4-bzImage -initrd rootfs.img -append "root=/dev/ram rdinit=sbin/init" -d func -D log
#/home/crdong/main/x86_32/qemu
