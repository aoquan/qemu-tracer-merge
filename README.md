一、安装qemu
=
目前QEMU的版本是2.0.2  
过程中可能需要安装pkg-config、libsdl1.2-dev、libpixman-1-dev、libfdt-dev、libtool、gcc-multilib等必要软件包  
./configure --target-list=i386-softmmu --enable-sdl进行配置，--target-list指定目标机的平台  
参见./configure --help  
然后执行make命令。如果是在64位机器上使用qemu-system-i386的话，最好使用make ARCH=x86_32,否则输出结果可能会不正常

在编译qemu的过程中,程序可能会报错,错误原因与系统的位数有关,在64为系统中unsigned int与long unsigned int的位数不相
等,而编译器将警告当作错误处理,导致编译失败.如果编译失败,可以在制定的代码出加上强制类型转换.(曹睿东可能会在仓库中建
不同的分支来解决这个问题)

二、安装简单系统busybox
=
下载busybox1.22.1源代码  
编译对应平台的busybox  
修改busybox目录下的Makefile  
第293行："LD =$(CC) -nostdlib " -->"LD =$(CC) -nostdlib -m32"  
参见configs/busybox/Makefile文件  
以x86_32为例  
make ARCH=i386 defconfig  
make ARCH=i386 menuconfig  
1.修改Busybox Settings->  
		Build Option ->   
			[*]Build Busybox as a static binary(no shared libs)  
2.[Busybox Settings]--->  
	[Build Options]--->  
		(-m32) Additional CFLAGS  
        (-m32) Additional LDFLAGS  
3.去掉不需要的功能，其它模块编译错误做法类似  
  Networking Utilities --->  
      [ ] inetd   
最终的配置文件参见configs/busybox/config文件  
4.make ARCH=i386  
5.make ARCH=i386 install

三、使用qemu模拟linux
=
1.编写initrd启动脚本，其中BUSYBOX是busybox的路径  
cd  BUSYBOX/_install  
mkdir -p proc sys dev etc/init.d  
vim BUSYBOX/_install/etc/init.d/rcS
```
	#!/bin/sh
	mount -t proc none /proc
	mount -t sysfs none /sys
	/sbin/mdev -s
```
参见configs/scripts/rcS文件  

2.制作rootfs.img镜像  
cd  BUSYBOX/_install  
find . | cpio -o --format=newc > BUSYBOX/rootfs.img  
注意rootfs.img不能生产在当前文件夹下，否则会生产自己的镜像。这里产生的镜像供下一步使用。
               
3.编写启动脚本  
首先要在工作的目录下新建一个addrs文件，存有5行地址，分别是内核起始地址，内核结束地址，busybox起始地址，busybox结束地址和内核符号表中modules的地址，可以参见configs/scripts/addrs文件，但需要注意的是每个内核镜像中的地址都不相同，需要自己查询。内核编译的过程是先生成一个带符号表的vmlinux文件，然后进行裁剪生成bzImage文件。qemu需要的是bzImage文件，但是查询符号表时要查询相应的vmlinux文件。可以使用nm -n命令查询符号表。busybox的地址可以在编译出的busybox_unstripped的符号表中查到（使用nm命令）。也可以查看内核和busybox编译出来生成的System.map，也含有符号表。内核起始地址就是_text的地址，内核结束地址是_end的地址，busybox
起始地址是_init的地址，busybox结束地址是_end的地址。还可以使用insert-busybox.rb脚本将busybox符号表导入数据库中。  

linux内核编译过程,如果编译为32位,则使用make ARCH=i386 defconfig,默认则使用make defconfig.

使用以下命令运行qemu，必须手动输入
``` 
	#!/bin/sh
	qemut-tracer/i386-softmmu/qemu-system-i386 -kernel bzImage -initrd rootfs.img -append "root=/dev/ram rdinit=sbin/init" -rtc clock=vm -icount 5 –d func –D log
``` 
其中bzImage是linux内核编译出的文件，在arch/i386/boot文件夹下，rootfs.img是上一步创建的镜像文件  
-d 参数指定日志输出项目,对qemu做了一定的修改,并增加了一个选项func可以追踪函数调用  
-D 参数指定日志输出文件  
针对自动测试的要求，已经写好了脚本，参见configs/scripts/a.sh
还需要对数据文件进行解析，包括parse.rb linux-3.5.4 x86_32 log result,然后使用self_time.py等脚本再处理，可以参见db-rtl-callgraph的帮助文档 

四、可加载模块
=
编译可加载模块，代码应满足一定的格式  
示例参见configs/module/factorial.c文件  
并要有Makefile，参见configs/module/Makefile文件   
在虚拟机中执行modprobe或insmod命令（具体使用可参考Linux使用帮助）载入模块

五、制作供QEMU使用的硬盘映像
=
1. 系统为64位Mint 17，内核版本为3.19.1，硬盘映像格式为raw  
2. 在http://www.linuxmint.com/release.php?id=22 下载Mint 17相应版本64位的iso  
3. 使用VirtualBox（也可用VMware，未经实验）新建一个硬盘映像，格式选择QCOW（或VDI，未经实验）  
4. 用刚才下载iso引导安装Mint 17系统  
5. 安装好的系统已包含测试需要的Firefox，Libre Office和VLC，还需要安装一些必要软件
6. 下载linux3.19.1源码，make menuconfig配置才能够共图形界面使用，make之后make modules_install,make install更新内核，可以把旧的内核都删掉。此内核文件要放到服务器上，保证版本一致
7. 安装sikuli，需要apt-get install libtesseract3
8. 开机图形界面启动后自动执行脚本：settings-sessions-新加一个启动项，command填写脚本路径即可
9. 将此硬盘映像上传至服务器，使用qemu-img convert -f qcow -O raw *.qcow xxx.raw命令转换为raw格式

六、制作供QEMU使用的硬盘映像（旧）
=
1. 系统为64位Mint 17，内核版本为3.19.1，硬盘映像格式为raw  
2. 在http://www.linuxmint.com/release.php?id=22 下载Mint 17相应版本64位的iso  
3. 使用VirtualBox（也可用VMware，未经实验）新建一个硬盘映像，格式最好选择QCOW（或VDI，未经实验）  
4. 用刚才下载iso引导安装Mint 17系统  
5. 安装好的系统已包含测试需要的Firefox，Libre Office和VLC，还需要安装一些必要软件，如vim  
6. 下载http://kernel.ubuntu.com/~kernel-ppa/mainline/v3.19.1-vivid/linux-headers-3.19.1-031901-generic_3.19.1-031901.201504091335_amd64.deb , http://kernel.ubuntu.com/~kernel-ppa/mainline/v3.19.1-vivid/linux-headers-3.19.1-031901_3.19.1-031901.201504091335_all.deb 和 http://kernel.ubuntu.com/~kernel-ppa/mainline/v3.19.1-vivid/linux-image-3.19.1-031901-generic_3.19.1-031901.201504091335_amd64.deb 三个deb包  
7. 使用sudo dpkg -i linux-*.deb命令更新内核，重启后可检查内核版本已更新  
8. 将此硬盘映像上传至服务器，使用qemu-img convert -f qcow -O raw *.qcow xxx.raw命令转换为raw格式，就可以使用了
