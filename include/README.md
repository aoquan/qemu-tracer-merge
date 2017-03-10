##how to add an include file in qemu
1. mkdir a dir in qemu-tracer/include/ eg: comm_struct ,put .h file into it
1. mkdir a same name dir in qemu-tracer/ eg: comm_struct, put .c file into it 
1. create a Makefile.objs in qemu-tracer/comm_struct, reference qemu-tracer/comm_struct/Makefile.objs
1. edit Makefile.objs of qemu-tracer, add a line "common-obj-y += comm_struct/"
