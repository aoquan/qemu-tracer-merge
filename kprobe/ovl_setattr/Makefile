PWD = $(shell pwd)

KDIR = /lib/modules/$(shell uname -r)/build

module := ovl_setattr

obj-m := $(module).o

TARGET = $(module).ko

all:
	make -C $(KDIR) M=$(PWD) modules

clean:
	make -C $(KDIR) M=$(PWD) clean

install:
	insmod ovl_setattr.ko

remove:
	rmmod ovl_setattr.ko
