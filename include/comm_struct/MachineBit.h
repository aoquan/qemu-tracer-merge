#define MACH_BIT
#include"stdint.h"
#define osBit32 1
#define isBusybox 1
#define isLubuntu32 0 
#define isLubuntu64 0
#define QEMULOG 1 //whether record the functions call log
#if osBit32 
typedef uint32_t my_target_ulong;
#define kernelMinAddr 0xc0000000           //if an address less than kernelMinAddr, it is in user space, otherwise in kernel space 
#define kernelMaxAddr 0xffffffff
#define gotMinAddr 0x80000000              //if the share object start address in 32 bit os
#define MY_TARGET_FMT_lx "%08x" 
#define MY_TARGET_lx "%x"
#else
typedef uint64_t my_target_ulong;
#define kernelMinAddr 0xf000000000000000
#define kernelMaxAddr 0xffffffffffffffff
#define gotMinAddr 0x0000555555554000 
#define MY_TARGET_FMT_lx "%016" PRIx64
#define MY_TARGET_lx "%" PRIx64
#endif

#if isLubuntu32
#define commOffset 0x3f0 
#define pidOffset 0x308 
#define tgidOffset 0x30c
#define parentOffset 0x318
#define realParentOffset 0x314
#endif

#if isLubuntu64
#define commOffset 0x5e0 //lubuntu64
#define pidOffset 0x438 
#define realParentOffset 0x448
#define parentOffset 0x448
#define tgidOffset 0x43c
#define moduleNameOffset 0x18
#define moduleCoreOffset 0x1c8
//do_init_module, linux-4.2.0
#define FUNC_PARA_MODULE 0xffffffff817e66a0
#define MODULE_NAME "overlay"
#endif

#if isBusybox
#define commOffset 0x2cc //busybox 
#define pidOffset 0x1f4
#define tgidOffset 0x1f8
#define realParentOffset 0x1fc
#define moduleNameOffset 0xc
#define moduleCoreOffset 0xc4
//trim_init_extable_addr, linux-3.5.4
#define FUNC_PARA_MODULE 0xc1025170
#define MODULE_NAME "offset"
#endif 

#define PARA_INT 0
#define PARA_STRING 1 
#define PARA_SOCKET 2 
#define PARA_CRED 3 
#define PARA_INODE 4
#define PARA_DENTRY 5


#define RECORD_FUNC_NO -1 //do not use this condition 
#define RECORD_SPEC_FUNC 1
#define RECORD_ALL_FUNC_WITHOUT_PARA 2
#define RECORD_ALL_FUNC_WITH_PARA 3
#define RECORD_PROCESS_FUNC_WITHOUT_PARA 4 
#define RECORD_PROCESS_FUNC_WITH_PARA 5
#define RECORD_OTHER 6

#define FUNC_MAX 300
#define PARAM_MAX 6
//my_target_ulong kernel_start,kernel_end,funcaddr[FUNC_MAX];
typedef struct{
    int pos;
    int reg;
    char type[20];
    int i_type; //using int to describe char type [20]
}Func_para;

typedef struct{
    my_target_ulong funcaddr;
    Func_para para[PARAM_MAX];
    Func_para ret;
    int para_num;
}Trace_func;
uint64_t hex2int(char s[]);
int upper2lower(int c);
