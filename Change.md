##qemu-tracer改动说明文档
###目前修改的地方
* configs.txt中加入了的第4行是程序的got（GLOBAL_OFFSET_TABLE）表地址，目的是为了方便程序出错时打印动态链接库的加载地址。
* 在vl.c中加入了got变量，用于存储got表的地址，用于32位系统的got使用的是uint_32 got；而64位系统使用的是target_ulong got；
* 在cpu-exec.c中加入了，printLinkMap（FILE * fp, CPUState cpu,target_ulong got）函数（64位），同时64位系统中的结构体link_map中的数据类型均为target_ulong，而在32位系统中采用的uint_32，所以输出的时候采用的占位符也会有相应的调整，fprintf()的占位符可能不一样；
* 对于64位系统，tr指向的TSS中的esp0会有一个0x4000的偏移（先减后压栈导致），需要减去0x4000，代码如下：
```
cpu_memory_rw_debug(cpu,esp0-0x4000,(uint8_t *)&task,sizeof(processname),0);
```
而在32位系统中，采用的是esp0&0xffffe000：
```
cpu_memory_rw_debug(cpu,esp0&0xffffe000,(uint8_t *)&task,sizeof(task),0);
```

* 针对每个系统（eg：lubuntu）需要计算task_struct的comm offset，计算的方法参考 https://github.com/caoruidong/comm-offset ，根据计算结果修改（大概位置629行）

```
cpu_memory_rw_debug(cpu,task+0x5e0,(uint8_t *)&processname,sizeof(processname),0);
```
其中，0x5e0便是comm offset;

* 64位系统和32位系统判断内核态还是用户态使用的地址不一样，64位使用esp<0xf000000000000000判断，而32位使用esp<0xc0000000判断；
* 632位系统需要对返回值的esp进行修正，
```
ld.esp = esp-sizeof(target_ulong)+4;
```
* 在32位系统中，由于ret可以带立即数，即ret im，表示ret之后，退栈im个，这会导致栈匹配算法失败，因此在32位系统中，位ret im的情况做了特别处理，即遇到时直接丢弃；
* 在32位系统中，新建进程时，第一个进程的第一条操作是返回，该返回的会返回到其父进程中，这时qemu-tracer还没有为其建立堆栈（遇到新的线程的call时新建堆栈），所以此时，直接丢弃；

##如何让64为qemu兼容32位操作系统，从而从而保证改动的一致性
* 在cpu-exec.c中加入osBit32，作为条件编译的条件，osBit32 == 1时，表示需要运行的系统为32位，否则为0
* 在使用新的内核后，需要更改参数commOffset，可以使用曹睿东的帮助，https://github.com/caoruidong/comm-offset 
* configure的时候统一使用configure --target-list=x86_64-softmmu
* 在ubuntu16.04上编译可能会有问题，因为一些函数过期，所以可以使用 --disable-werror 选项
