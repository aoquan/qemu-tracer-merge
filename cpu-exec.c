/*
 *  emulator main execution loop
 *
 *  Copyright (c) 2003-2005 Fabrice Bellard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, see <http://www.gnu.org/licenses/>.
 */
#include "config.h"
#include "cpu.h"
#include "trace.h"
#include "disas/disas.h"
#include "tcg.h"
#include "qemu/atomic.h"
#include "sysemu/qtest.h"
#include "qemu/timer.h"
#include "exec/address-spaces.h"
#include "exec/memory-internal.h"
#include "qemu/rcu.h"
#include "exec/tb-hash.h"


#include "include/comm_struct/List.h"
//merge all function
#define RECORD_ALL_CALL_RET 0
#define RECORD_USER_CALL_RET 0
#define RECORD_KERNEL_CALL_RET 0
#define RECORD_FUNC_STACK 0
#define RECORD_FUNC_PARAMETERS 0
#define RECORD_SPECIFIC_FUNC 0
#define RECORD_SPECIFIC_PROGRAM 0
int record_what;
target_ulong task;
//end merge


/* -icount align implementation. */

typedef struct SyncClocks {
    int64_t diff_clk;
    int64_t last_cpu_icount;
    int64_t realtime_clock;
} SyncClocks;

#if !defined(CONFIG_USER_ONLY)
/* Allow the guest to have a max 3ms advance.
 * The difference between the 2 clocks could therefore
 * oscillate around 0.
 */
#define VM_CLOCK_ADVANCE 3000000
#define THRESHOLD_REDUCE 1.5
#define MAX_DELAY_PRINT_RATE 2000000000LL
#define MAX_NB_PRINTS 100

/////////////////////////////////////////
static int printList(threadList* a,void *e){
    while(!isStackEmpty(a->stack)){
        logData ld;
        popStack(a->stack,&ld);
        // printf("##: %c  ",ld.type);
        my_qemu_log("C,%s,"TARGET_FMT_lx","TARGET_FMT_lx","TARGET_FMT_lx",%d,"TARGET_FMT_lx"\n",ld.processName,ld.curAddr,ld.goAddr,ld.pid,ld.tid,ld.esp);
    }
    my_qemu_log("__________________________\n");
    return 0;
}
/*
static int printPidList(my_target_ulong * a,void *e){
    printf("%d     ",(int)*a);
    return 0;
}
*/
//////////////////////////////////////


static void align_clocks(SyncClocks *sc, const CPUState *cpu)
{
    int64_t cpu_icount;

    if (!icount_align_option) {
        return;
    }

    cpu_icount = cpu->icount_extra + cpu->icount_decr.u16.low;
    sc->diff_clk += cpu_icount_to_ns(sc->last_cpu_icount - cpu_icount);
    sc->last_cpu_icount = cpu_icount;

    if (sc->diff_clk > VM_CLOCK_ADVANCE) {
#ifndef _WIN32
        struct timespec sleep_delay, rem_delay;
        sleep_delay.tv_sec = sc->diff_clk / 1000000000LL;
        sleep_delay.tv_nsec = sc->diff_clk % 1000000000LL;
        if (nanosleep(&sleep_delay, &rem_delay) < 0) {
            sc->diff_clk = rem_delay.tv_sec * 1000000000LL + rem_delay.tv_nsec;
        } else {
            sc->diff_clk = 0;
        }
#else
        Sleep(sc->diff_clk / SCALE_MS);
        sc->diff_clk = 0;
#endif
    }
}

static void print_delay(const SyncClocks *sc)
{
    static float threshold_delay;
    static int64_t last_realtime_clock;
    static int nb_prints;

    if (icount_align_option &&
            sc->realtime_clock - last_realtime_clock >= MAX_DELAY_PRINT_RATE &&
            nb_prints < MAX_NB_PRINTS) {
        if ((-sc->diff_clk / (float)1000000000LL > threshold_delay) ||
                (-sc->diff_clk / (float)1000000000LL <
                 (threshold_delay - THRESHOLD_REDUCE))) {
            threshold_delay = (-sc->diff_clk / 1000000000LL) + 1;
            printf("Warning: The guest is now late by %.1f to %.1f seconds\n",
                    threshold_delay - 1,
                    threshold_delay);
            nb_prints++;
            last_realtime_clock = sc->realtime_clock;
        }
    }
}

static void init_delay_params(SyncClocks *sc,
        const CPUState *cpu)
{
    if (!icount_align_option) {
        return;
    }
    sc->realtime_clock = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL_RT);
    sc->diff_clk = qemu_clock_get_ns(QEMU_CLOCK_VIRTUAL) - sc->realtime_clock;
    sc->last_cpu_icount = cpu->icount_extra + cpu->icount_decr.u16.low;
    if (sc->diff_clk < max_delay) {
        max_delay = sc->diff_clk;
    }
    if (sc->diff_clk > max_advance) {
        max_advance = sc->diff_clk;
    }

    /* Print every 2s max if the guest is late. We limit the number
       of printed messages to NB_PRINT_MAX(currently 100) */
    print_delay(sc);
}
#else
static void align_clocks(SyncClocks *sc, const CPUState *cpu)
{
}

static void init_delay_params(SyncClocks *sc, const CPUState *cpu)
{
}
#endif /* CONFIG USER ONLY */

void cpu_loop_exit(CPUState *cpu)
{
    cpu->current_tb = NULL;
    siglongjmp(cpu->jmp_env, 1);
}

/* exit the current TB from a signal handler. The host registers are
   restored in a state compatible with the CPU emulator
 */
#if defined(CONFIG_SOFTMMU)
void cpu_resume_from_signal(CPUState *cpu, void *puc)
{
    /* XXX: restore cpu registers saved in host registers */

    cpu->exception_index = -1;
    siglongjmp(cpu->jmp_env, 1);
}

void cpu_reload_memory_map(CPUState *cpu)
{
    AddressSpaceDispatch *d;

    if (qemu_in_vcpu_thread()) {
        /* Do not let the guest prolong the critical section as much as it
         * as it desires.
         *
         * Currently, this is prevented by the I/O thread's periodinc kicking
         * of the VCPU thread (iothread_requesting_mutex, qemu_cpu_kick_thread)
         * but this will go away once TCG's execution moves out of the global
         * mutex.
         *
         * This pair matches cpu_exec's rcu_read_lock()/rcu_read_unlock(), which
         * only protects cpu->as->dispatch.  Since we reload it below, we can
         * split the critical section.
         */
        rcu_read_unlock();
        rcu_read_lock();
    }

    /* The CPU and TLB are protected by the iothread lock.  */
    d = atomic_rcu_read(&cpu->as->dispatch);
    cpu->memory_dispatch = d;
    tlb_flush(cpu, 1);
}
#endif

/* Execute a TB, and fix up the CPU state afterwards if necessary */
static inline tcg_target_ulong cpu_tb_exec(CPUState *cpu, uint8_t *tb_ptr)
{
    CPUArchState *env = cpu->env_ptr;
    uintptr_t next_tb;

#if defined(DEBUG_DISAS)
    if (qemu_loglevel_mask(CPU_LOG_TB_CPU)) {
#if defined(TARGET_I386)
        log_cpu_state(cpu, CPU_DUMP_CCOP);
#elif defined(TARGET_M68K)
        /* ??? Should not modify env state for dumping.  */
        cpu_m68k_flush_flags(env, env->cc_op);
        env->cc_op = CC_OP_FLAGS;
        env->sr = (env->sr & 0xffe0) | env->cc_dest | (env->cc_x << 4);
        log_cpu_state(cpu, 0);
#else
        log_cpu_state(cpu, 0);
#endif
    }
#endif /* DEBUG_DISAS */

    cpu->can_do_io = !use_icount;
    next_tb = tcg_qemu_tb_exec(env, tb_ptr);
    cpu->can_do_io = 1;
    trace_exec_tb_exit((void *) (next_tb & ~TB_EXIT_MASK),
            next_tb & TB_EXIT_MASK);

    if ((next_tb & TB_EXIT_MASK) > TB_EXIT_IDX1) {
        /* We didn't start executing this TB (eg because the instruction
         * counter hit zero); we must restore the guest PC to the address
         * of the start of the TB.
         */
        CPUClass *cc = CPU_GET_CLASS(cpu);
        TranslationBlock *tb = (TranslationBlock *)(next_tb & ~TB_EXIT_MASK);
        if (cc->synchronize_from_tb) {
            cc->synchronize_from_tb(cpu, tb);
        } else {
            assert(cc->set_pc);
            cc->set_pc(cpu, tb->pc);
        }
    }
    if ((next_tb & TB_EXIT_MASK) == TB_EXIT_REQUESTED) {
        /* We were asked to stop executing TBs (probably a pending
         * interrupt. We've now stopped, so clear the flag.
         */
        cpu->tcg_exit_req = 0;
    }
    return next_tb;
}

/* Execute the code without caching the generated code. An interpreter
   could be used if available. */
static void cpu_exec_nocache(CPUState *cpu, int max_cycles,
        TranslationBlock *orig_tb)
{
    TranslationBlock *tb;

    /* Should never happen.
       We only end up here when an existing TB is too long.  */
    if (max_cycles > CF_COUNT_MASK)
        max_cycles = CF_COUNT_MASK;

    tb = tb_gen_code(cpu, orig_tb->pc, orig_tb->cs_base, orig_tb->flags,
            max_cycles | CF_NOCACHE);
    tb->orig_tb = tcg_ctx.tb_ctx.tb_invalidated_flag ? NULL : orig_tb;
    cpu->current_tb = tb;
    /* execute the generated code */
    qemu_log("nocache "TARGET_FMT_lx"\n",tb->pc);
    trace_exec_tb_nocache(tb, tb->pc);
    cpu_tb_exec(cpu, tb->tc_ptr);
    cpu->current_tb = NULL;
    tb_phys_invalidate(tb, -1);
    tb_free(tb);
}

static TranslationBlock *tb_find_slow(CPUState *cpu,
        target_ulong pc,
        target_ulong cs_base,
        uint64_t flags)
{
    CPUArchState *env = (CPUArchState *)cpu->env_ptr;
    TranslationBlock *tb, **ptb1;
    unsigned int h;
    tb_page_addr_t phys_pc, phys_page1;
    target_ulong virt_page2;

    tcg_ctx.tb_ctx.tb_invalidated_flag = 0;

    /* find translated block using physical mappings */
    phys_pc = get_page_addr_code(env, pc);
    phys_page1 = phys_pc & TARGET_PAGE_MASK;
    h = tb_phys_hash_func(phys_pc);
    ptb1 = &tcg_ctx.tb_ctx.tb_phys_hash[h];
    for(;;) {
        tb = *ptb1;
        if (!tb)
            goto not_found;
        if (tb->pc == pc &&
                tb->page_addr[0] == phys_page1 &&
                tb->cs_base == cs_base &&
                tb->flags == flags) {
            /* check next page if needed */
            if (tb->page_addr[1] != -1) {
                tb_page_addr_t phys_page2;

                virt_page2 = (pc & TARGET_PAGE_MASK) +
                    TARGET_PAGE_SIZE;
                phys_page2 = get_page_addr_code(env, virt_page2);
                if (tb->page_addr[1] == phys_page2)
                    goto found;
            } else {
                goto found;
            }
        }
        ptb1 = &tb->phys_hash_next;
    }
not_found:
    /* if no translated code available, then translate it now */
    tb = tb_gen_code(cpu, pc, cs_base, flags, 0);

found:
    /* Move the last found TB to the head of the list */
    if (likely(*ptb1)) {
        *ptb1 = tb->phys_hash_next;
        tb->phys_hash_next = tcg_ctx.tb_ctx.tb_phys_hash[h];
        tcg_ctx.tb_ctx.tb_phys_hash[h] = tb;
    }
    /* we add the TB in the virtual pc hash table */
    cpu->tb_jmp_cache[tb_jmp_cache_hash_func(pc)] = tb;
    return tb;
}

static inline TranslationBlock *tb_find_fast(CPUState *cpu)
{
    CPUArchState *env = (CPUArchState *)cpu->env_ptr;
    TranslationBlock *tb;
    target_ulong cs_base, pc;
    int flags;

    /* we record a subset of the CPU state. It will
       always be the same before a given translated block
       is executed. */
    cpu_get_tb_cpu_state(env, &pc, &cs_base, &flags);
    tb = cpu->tb_jmp_cache[tb_jmp_cache_hash_func(pc)];
    if (unlikely(!tb || tb->pc != pc || tb->cs_base != cs_base ||
                tb->flags != flags)) {
        tb = tb_find_slow(cpu, pc, cs_base, flags);
    }
    return tb;
}

static void cpu_handle_debug_exception(CPUState *cpu)
{
    CPUClass *cc = CPU_GET_CLASS(cpu);
    CPUWatchpoint *wp;

    if (!cpu->watchpoint_hit) {
        QTAILQ_FOREACH(wp, &cpu->watchpoints, entry) {
            wp->flags &= ~BP_WATCHPOINT_HIT;
        }
    }

    cc->debug_excp_handler(cpu);
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static inline my_target_ulong getParentPid(CPUState *cpu,my_target_ulong realParentAddr){
    my_target_ulong pts=0 ; //parent task_struct
    my_target_ulong ppid=0; //parent pid 
    my_target_ulong tmp=0;
    cpu_memory_rw_debug(cpu,realParentAddr,(uint8_t*)&pts,sizeof(tmp),0);
    cpu_memory_rw_debug(cpu,pts+pidOffset,(uint8_t*)&ppid,sizeof(ppid),0);
    return ppid;
}

static inline my_target_ulong getPid(CPUState *cpu,my_target_ulong pidAddr){
    my_target_ulong pid;
    cpu_memory_rw_debug(cpu,pidAddr,(uint8_t*)&pid,sizeof(pid),0);
    return pid;
}


// get ip and port

struct sockaddr{
    unsigned short sa_family; /* 地址族, AF_xxx */
    char sa_data[14]; /* 14字节的协议地址*/
};
struct mySocket{
    unsigned short sa_family;
    int port;
    //int ip[4];
    unsigned char ip[4];
};

static struct mySocket printSocket(FILE * fp, CPUState *cpu,my_target_ulong rsi){
    struct mySocket ms={0,0,{0}};
    struct sockaddr sa;
    cpu_memory_rw_debug(cpu,rsi,(uint8_t *)&sa,sizeof(sa),0);
////////
    if(sa.sa_family!=2){
        fprintf(fp,"S,%d,%d,%d.%d.%d.%d\n",sa.sa_family,ms.port,ms.ip[0],ms.ip[1],ms.ip[2],ms.ip[3]);
        return ms;
    }
    else{
        /*
           for(int i=0;i<14;i++){
           fprintf(fp,"%d ",sa.sa_data[i]);
           }
           fprintf(fp,"\n");
         */
        ms.sa_family=sa.sa_family;
        int data0,data1;
        if(sa.sa_data[0]<0) data0=0x100+sa.sa_data[0];
        else data0 = sa.sa_data[0];
        if(sa.sa_data[1]<0) data1=0x100+sa.sa_data[1];
        else data1 = sa.sa_data[1];
        ms.port = data0*0x100 + data1;
        ms.ip[0] = sa.sa_data[2];
        ms.ip[1] = sa.sa_data[3];
        ms.ip[2] = sa.sa_data[4];
        ms.ip[3] = sa.sa_data[5];
        //fprintf(fp,"sf_family=%d,port=%d,ip=%d.%d.%d.%d\n",ms.sa_family,ms.port,ms.ip[0],ms.ip[1],ms.ip[2],ms.ip[3]);
        fprintf(fp,"S,%d,%d,%d.%d.%d.%d\n",ms.sa_family,ms.port,ms.ip[0],ms.ip[1],ms.ip[2],ms.ip[3]);
        return ms;
    }
}

static int print_inode(FILE * fp,CPUState *cpu,my_target_ulong d_inode){
    int i_mode_offset = 0x0;
    int i_uid_offset = 0x4;
    int i_gid_offset = 0x8;
    int i_ino_offset;

#if isLubuntu64
    i_ino_offset = 0x40;
#endif

#if isBusybox
    i_ino_offset = 0x28;
#endif

    my_target_ulong i_mode,i_uid,i_gid,i_ino;
//    target_ulong i_ino;
    cpu_memory_rw_debug(cpu,d_inode+i_mode_offset,(uint8_t *)&i_mode,sizeof(i_mode),0);
    cpu_memory_rw_debug(cpu,d_inode+i_uid_offset,(uint8_t *)&i_uid,sizeof(i_uid),0);
    cpu_memory_rw_debug(cpu,d_inode+i_gid_offset,(uint8_t *)&i_gid,sizeof(i_gid),0);
    cpu_memory_rw_debug(cpu,d_inode+i_ino_offset,(uint8_t *)&i_ino,sizeof(i_ino),0);
//    fprintf(fp,"file inode, i_ino:"TARGET_FMT_ld", mode:%o, uid:%x, gid:%x||",i_ino,(short)i_mode,(int)i_uid,(int)i_gid);
    fprintf(fp,"file inode, i_ino:%d, mode:%o, uid:%x, gid:%x||",(int)i_ino,(short)i_mode,(int)i_uid,(int)i_gid);
    return 0;
}
/*
    record mode, uid, gid of an file when chmod
*/

static void print_file_inode_by_dentry(FILE * fp,CPUState *cpu,my_target_ulong dentry){
    int d_inode_offset;
/*  lubuntu64
*/
#if isLubuntu64
    d_inode_offset = 0x30;
#endif

#if isBusybox
    d_inode_offset = 0x20;
#endif
    my_target_ulong d_inode;
    cpu_memory_rw_debug(cpu,dentry+d_inode_offset,(uint8_t *)&d_inode,sizeof(d_inode),0);
    print_inode(fp,cpu,d_inode);
}


static int print_cred_by_task_struct(FILE * fp,CPUState *cpu,my_target_ulong task){
int cred_offset;
#if isLubuntu64
    cred_offset = 0x5d8;
#endif

#if isBusybox
    cred_offset = 0x2c8;
#endif

    int uid_offset = 0x4;
    int gid_offset = 0x8;

    my_target_ulong cred,uid,gid;
    cpu_memory_rw_debug(cpu,task+cred_offset,(uint8_t *)&cred,sizeof(cred),0);
    cpu_memory_rw_debug(cpu,cred+uid_offset,(uint8_t *)&uid,sizeof(uid),0);
    cpu_memory_rw_debug(cpu,cred+gid_offset,(uint8_t *)&gid,sizeof(gid),0);
    fprintf(fp,"task struct cred info, uid:%d, gid:%d||",(int)uid,(int)gid);
    return 0;
}


struct link_map{ 
    my_target_ulong l_addr;
    my_target_ulong l_name;
    my_target_ulong l_ld;
    my_target_ulong l_next;
};

struct pt_dynamic{
    my_target_ulong d_tag;
    my_target_ulong d_un;
};

//get start address of link map using .got.plt section address
static my_target_ulong getLinkMapStartAddrByGot(CPUState *cpu, my_target_ulong got){
    my_target_ulong plinkmap;
    cpu_memory_rw_debug(cpu,got+sizeof(my_target_ulong),(uint8_t *)&plinkmap,sizeof(plinkmap),0);
    return plinkmap;
}


//get start address of link map using .dynamic section address
static my_target_ulong getLinkMapStartAddrByDynamic(FILE * fp,CPUState *cpu, my_target_ulong dynamicSectionAddr){
    //target_ulong startAddr = 0x0000555555554000 + dynamicSectionAddr;
    my_target_ulong startAddr =  dynamicSectionAddr; //.dynamic + 0x0000555555554000
    struct pt_dynamic ptd;
    int i;
    for( i=0;i<85;i++){
        cpu_memory_rw_debug(cpu,startAddr+i*2*sizeof(my_target_ulong),(uint8_t *)&ptd,sizeof(ptd),0);
        fprintf(fp,TARGET_FMT_lx" "MY_TARGET_FMT_lx" "MY_TARGET_FMT_lx"\n",startAddr+i*2*sizeof(my_target_ulong),ptd.d_tag,ptd.d_un);
        if(ptd.d_tag == 0x15){
            struct pt_dynamic re ;
            cpu_memory_rw_debug(cpu,ptd.d_un,(uint8_t *)&re,sizeof(re),0);
            return re.d_un;
        }
    }
    return 0;
}

// tarverse the link map and print the load address and names of SO 
static void traverseLinkMap(FILE *fp ,CPUState *cpu,my_target_ulong plinkmap){
    struct link_map lnk_tmp;
    cpu_memory_rw_debug(cpu,plinkmap,(uint8_t *)&lnk_tmp,sizeof(lnk_tmp),0);
    cpu_memory_rw_debug(cpu,lnk_tmp.l_next,(uint8_t *)&lnk_tmp,sizeof(lnk_tmp),0);
    char soname[50];
    fprintf(fp,"linkmap begin\n");
    int i=0;
    while(lnk_tmp.l_next!=0){                  
        cpu_memory_rw_debug(cpu,lnk_tmp.l_next,(uint8_t *)&lnk_tmp,sizeof(lnk_tmp),0);
        cpu_memory_rw_debug(cpu,lnk_tmp.l_name,(uint8_t *)&soname,sizeof(soname),0);
#if osBit32
        fprintf(fp,"%s %I32u\n",soname,lnk_tmp.l_addr);
#else
        fprintf(fp,"%s "TARGET_FMT_lx"\n",soname,lnk_tmp.l_addr);
#endif
        if(i>=10) break;
        i++; 
    }           
    fprintf(fp,"linkmap end\n");
}

static void printLinkMap(FILE * fp,CPUState *cpu,my_target_ulong got){
    //target_ulong plinkmap = getLinkMapStartAddrByGot(cpu,got);
    my_target_ulong plinkmap;
    if(got>gotMinAddr){
        plinkmap = getLinkMapStartAddrByDynamic(fp,cpu,got);
        if(plinkmap ==0 ){
            printf("can not find link map by .dynamic\n");
            return ;
        }
    }else{
        plinkmap = getLinkMapStartAddrByGot(cpu,got);
        //plinkmap = getLinkMapStartAddrByDynamic(fp,cpu,got);
    }
    traverseLinkMap(fp,cpu,plinkmap);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//extern my_target_ulong kernel_start,kernel_end,funcaddr[];
/*
extern my_target_ulong funcaddr[];
extern int funcParaPos[],funcParaType[];
extern char funcargv[][6],target[];
*/

extern Trace_func trace_func[];


extern int funccount;
//const int argorder[6]={R_EDI,R_ESI,R_EDX,R_ECX,8,9};
extern target_ulong got;
//extern target_ulong ptDynamic;
extern bool print_funcstack;
extern List  program_list;
extern my_target_ulong  kernel_addr_begin,kernel_addr_end,user_addr_begin,user_addr_end;
extern int is_record_kernel,is_record_user,trace_type;

bool output=false;
////////////////////////////////////////////////////
//added by aquan
List L;
List tracePidList;
List retAddrList;
my_target_ulong retAddrTmp;
bool is_list_init = false;
threadList* curThread;
Stack* s;
threadList* tl ;
logData ld,ldPop;
logData ld_global,ldPop_global;
int countCpuExec = 0;
FILE *stackWrite;
pthread_mutex_t singleton_lock=PTHREAD_MUTEX_INITIALIZER; 
bool prev_is_ret = false;

//singleton model
static void get_write_file(void){
    pthread_mutex_lock(&singleton_lock);
    if(!stackWrite){
        stackWrite = fopen("stack","w");
        if(NULL == stackWrite){
            pthread_mutex_unlock(&singleton_lock);
            exit(0);
        }
    }
    pthread_mutex_unlock(&singleton_lock);
}
struct ret_struct{
    my_target_ulong address;
    int ret_type;
};

static void init_some_list(void){
    if(!is_list_init){
        initList(&retAddrList,sizeof(struct ret_struct));
        is_list_init = true;
    }
}
//added by aquan
////////////////////////////////////////////////////

static int funcistraced(my_target_ulong target)
{
    int low=0,high=funccount-1,mid;
    while(low<=high){
        mid=(low+high)>>1;
//        if(funcaddr[mid]>target)
        if(trace_func[mid].funcaddr > target)
            high=mid-1;
//        else if(funcaddr[mid]<target)
        else if(trace_func[mid].funcaddr < target)
            low=mid+1;
        else
            return mid;
    }
    return -1;
}


//get parameter
static inline void printStrParameter(FILE * fp, CPUState *cpu,my_target_ulong reg){
    char para[50]={0};
    cpu_memory_rw_debug(cpu,reg,(uint8_t *)&para,sizeof(para),0);
    fprintf(fp,"%s||",para);
    return;
}

static inline void printIntParameter(FILE * fp,my_target_ulong reg){
    fprintf(fp,MY_TARGET_FMT_lx"||",reg);
}

static void print_reg(FILE *fp,CPUState *cpu, my_target_ulong reg, int i_type){
    switch(i_type){
        case PARA_SOCKET :
            printSocket(stackWrite,cpu,reg);
            break;
        case PARA_STRING :
            printStrParameter(stackWrite,cpu,reg);
            break;
        case PARA_INT :
            printIntParameter(stackWrite,reg);
            break;
        case PARA_CRED :
            print_cred_by_task_struct(stackWrite,cpu,task);
            break;
        case PARA_INODE :
            print_inode(stackWrite,cpu,reg);
            break;
        case PARA_DENTRY :
            print_cred_by_task_struct(stackWrite,cpu,task);
            print_file_inode_by_dentry(stackWrite,cpu,reg);
            break;
    }
}

static void print_parameter(FILE *fp,CPUArchState *env,CPUState *cpu,int funcIndex){
    int i;
    for(i=1;i<trace_func[funcIndex].para_num;i++){
        my_target_ulong reg  =  env->regs[trace_func[funcIndex].para[i].reg];
        print_reg(fp, cpu, reg, trace_func[funcIndex].para[i].i_type);
    }
    fprintf(fp,"\n");
}

static void print_return(FILE *fp,CPUState * cpu,my_target_ulong eax,logData ld, int index){
    fprintf(fp,"%c,%s,"TARGET_FMT_lx","TARGET_FMT_lx",%d,%d,"TARGET_FMT_lx"||",ld.type,ld.processName,ld.curAddr,ld.goAddr,(int)ld.pid,ld.tid,ld.esp);
    print_reg(fp, cpu, eax, trace_func[index].para[0].i_type);
    fprintf(fp,"\n");
}

//record return value without parameter
/*
static void print_return_without_param(FILE *fp,logData ld){
    fprintf(fp,"%c,%s,"TARGET_FMT_lx","TARGET_FMT_lx",%d,%d,"TARGET_FMT_lx"||",ld.type,ld.processName,ld.curAddr,ld.goAddr,(int)ld.pid,ld.tid,ld.esp);
    fprintf(fp,"\n");
}
*/

static void print_all_regs_para(CPUArchState *env){
#if osBit32
    my_qemu_log(TARGET_FMT_lx" "TARGET_FMT_lx" "TARGET_FMT_lx"\n",env->regs[R_EAX],env->regs[R_ECX],env->regs[R_EDX]);
#else
    my_qemu_log(MY_TARGET_FMT_lx" "MY_TARGET_FMT_lx" "MY_TARGET_FMT_lx" "MY_TARGET_FMT_lx" "MY_TARGET_FMT_lx" "MY_TARGET_FMT_lx"\n",env->regs[R_EDI],env->regs[R_ESI],env->regs[R_EDX],env->regs[R_ECX],env->regs[8],env->regs[9]);
#endif
}


static inline void print_log_to_file(const logData ld){
    my_qemu_log("%c,%s,"TARGET_FMT_lx","TARGET_FMT_lx",%d,%d,"TARGET_FMT_lx"\n",ld.type,ld.processName,ld.curAddr,ld.goAddr,(int)ld.pid,ld.tid,ld.esp);
}
static inline void print_stack_to_file(FILE * fp, const logData ld){
    fprintf(fp,"%c,%s,"TARGET_FMT_lx","TARGET_FMT_lx",%d,%d,"TARGET_FMT_lx"\n",ld.type,ld.processName,ld.curAddr,ld.goAddr,(int)ld.pid,ld.tid,ld.esp);
}


static void record_stack_call(CPUArchState *env,CPUState *cpu,const logData ld){
   int funcIndex = funcistraced(ld.goAddr);
   if((ld.esp > kernelMinAddr) && funcIndex!=-1){
       print_log_to_file(ld);
       //print stack
       if(curThread->pid!=ld.pid || curThread->tid!=ld.tid){
           if(GetCurThread(&L,ld.pid,ld.tid,curThread)!=0){
               my_qemu_log("error! syscall can not find matched pid and tid.\n");
           }
       }

       print_parameter(stackWrite,env,cpu,funcIndex);
       if(funcIndex!=-1) print_parameter(stackWrite,env,cpu,funcIndex);
       print_stack_to_file(stackWrite,ld);
       void *stackTop = curThread->stack->pTop;
       logData ldTmp;
       while(!isStackEmpty(curThread->stack)){
           popStack(curThread->stack,&ldTmp);
           print_stack_to_file(stackWrite,ldTmp);
       }
       fprintf(stackWrite,"\n");
       curThread->stack->pTop = stackTop;
       if(got!=0){
           printLinkMap(stackWrite,cpu,got);
       }
   }

   if(ld.esp<kernelMinAddr){
       if(ld.pid==curThread->pid && ld.tid == curThread->tid){
           pushStack(curThread->stack,&ld);
       }
       else{
           //search threadList
           if(GetCurThread(&L,ld.pid,ld.tid,curThread)==0){
               //find this thread
               pushStack(curThread->stack,&ld);
           }
           else{
               //can't find this thread, need add a item to List
               s = malloc(sizeof(Stack));                  
               tl = malloc(sizeof(threadList));
               initStack(s,sizeof(logData));
               pushStack(s,&ld);
               tl->stack = s;
               tl->pid = ld.pid;
               tl->tid = ld.tid;

               appendList(&L,tl);
               curThread = tl;
           }
       }
       print_log_to_file(ld);
   }

}
static void record_stack_ret(const logData ld){
    if(ld.esp<kernelMinAddr){
        //pop stack
        int noPopTag = 0;

        if(ld.pid==curThread->pid && ld.tid == curThread->tid){
            //pop stack
            GetTopStack(curThread->stack,&ldPop);
            if(ldPop.esp != ld.esp){
                //exception: esp is not matched
                if(isStackEmpty(curThread->stack)){
                    my_qemu_log("fork return\n");
                }
                else{
                    my_qemu_log("pop stack while esp not matched,start\n");
                    while(!isStackEmpty(curThread->stack)){
                        GetTopStack(curThread->stack,&ldPop);
                        if(ld.esp==ldPop.esp || ld.goAddr-2 == ldPop.curAddr){
                            break;
                        }
                        else{
                            if(ld.esp<ldPop.esp){
                                my_qemu_log("abort this return @@\n");
                                noPopTag = 1;
                                break;
                            }
                            popStack(curThread->stack,&ldPop);
                            print_log_to_file(ldPop);
                        }
                    }
                    my_qemu_log("pop stack while esp not matched,end\n");

                    if(isStackEmpty(curThread->stack)){
                        my_qemu_log("algorithm error\n");
                        my_qemu_log("##########################################################\n");
                        print_log_to_file(ld);
                        my_qemu_log("**********************************************************\n");
                        traverseList(&L,(TRAVERSEFUNC)printList,0);
                        my_qemu_log("**********************************************************\n");

                       //printLinkMap(stackWrite,cpu,got);
                        exit(0);
                    }
                }

                if(noPopTag==0)
                    popStack(curThread->stack,&ldPop);
            }
            else{
                if(noPopTag==0)
                    popStack(curThread->stack,&ldPop);
            }
        }
        else{
            if(GetCurThread(&L,ld.pid,ld.tid,curThread)==0){
                //find this thread
                GetTopStack(curThread->stack,&ldPop);
                if(ldPop.esp != ld.esp){
                    if(isStackEmpty(curThread->stack)){
                        my_qemu_log("fork return2\n");
                    }
                    else{
                        my_qemu_log("pop stack while esp not matched,start1\n");
                        while(!isStackEmpty(curThread->stack)){
                            GetTopStack(curThread->stack,&ldPop);
                            if(ld.esp==ldPop.esp || ld.goAddr-2 == ldPop.curAddr){
                                break;
                            }
                            else{
                                if(ld.esp<ldPop.esp){
                                    my_qemu_log("abort this return @@\n");
                                    noPopTag = 1;
                                    break;
                                }
                                popStack(curThread->stack,&ldPop);
                                print_log_to_file(ldPop);
                            }
                        }
                        my_qemu_log("pop stack while esp not matched,end2\n");

                        if(isStackEmpty(curThread->stack)){
                            my_qemu_log("algorithm error2\n");
                            my_qemu_log("##########################################################\n");
                            print_log_to_file(ld);
                            my_qemu_log("**********************************************************\n");
                            traverseList(&L,(TRAVERSEFUNC)printList,0);
                            my_qemu_log("**********************************************************\n");
                            //printLinkMap(stackWrite,cpu,got);
                            exit(0);
                        }
                    }

                    if(noPopTag==0)
                        popStack(curThread->stack,&ldPop);
                }
                else{
                    if(noPopTag==0)
                        popStack(curThread->stack,&ldPop);
                }
            }
            else{
                //ERRER: can't find a thread which contains this return
                my_qemu_log("**********************************************************\n");
                my_qemu_log("error! a new thread but the fisrt Instruction is return \n");
                print_log_to_file(ld);
                //traverseList(&L,(TRAVERSEFUNC)printList,0);
                my_qemu_log("**********************************************************\n");
                //exit(0);
            }
        }
        //added by aquan
        print_log_to_file(ld);
    }

}

static void record_stack(CPUArchState *env,CPUState *cpu,const logData ld,int inListFlag){
    if(countCpuExec==0){
        initList(&L,sizeof(threadList));
        initList(&tracePidList,sizeof(int));
        appendList(&tracePidList,&ld.tid); // the first pid ,parent of all other process 
        curThread = malloc(sizeof(threadList));
        /*
        if(!stackWrite){
            stackWrite = fopen("stack","w");
            if(NULL == stackWrite){
                exit(0);
            }
        }
        */
        get_write_file();
        fprintf(stackWrite,"P,%d,%d,%d\n",(int)ld.ppid,(int)ld.tid,(int)ld.pid);
    }
    if(countCpuExec == 1 && inListFlag != -1){
        if(IndexOf(&tracePidList,ld.tid)==-1){
            fprintf(stackWrite,"P,%d,%d,%d,%s\n",(int)ld.ppid,(int)ld.tid,(int)ld.pid,ld.processName);
            appendList(&tracePidList,&ld.tid);
        }
    }
    countCpuExec = 1;
    ////
    if(cpu->current_tb->type==TB_CALL){
        record_stack_call(env,cpu,ld);
    }
    else{
        if(cpu->current_tb->type==TB_RET){
            record_stack_ret(ld);
        }
    }

}
static logData get_logdata(CPUArchState *env,CPUState *cpu,TranslationBlock *tb){
    target_ulong tr=env->tr.base,esp0;
    char processname[16];
//    memset(processname,0,sizeof(processname));
    cpu_memory_rw_debug(cpu,tr+0x4,(uint8_t *)&esp0,sizeof(esp0),0);
#if osBit32
    cpu_memory_rw_debug(cpu,esp0&0xffffe000,(uint8_t *)&task,sizeof(task),0);
#else
    cpu_memory_rw_debug(cpu,esp0-0x4000,(uint8_t *)&task,sizeof(task),0);
#endif
    cpu_memory_rw_debug(cpu,task+commOffset,(uint8_t *)&processname,sizeof(processname),0);
    my_target_ulong ppid = getParentPid(cpu,task+realParentOffset);
    my_target_ulong tgid = getPid(cpu,task+tgidOffset); //get tgid 
/////
    cpu->current_tb = tb;
    logData ld;
    ld.processName = processname;
    ld.goAddr = env->eip;
    //ld.pid = env->cr[3];
    ld.pid = (int)tgid;
    ld.ppid =(int)ppid;
    uint32_t tid;
    cpu_memory_rw_debug(cpu,task+pidOffset,(uint8_t *)&tid,sizeof(tid),0);
    ld.tid = tid;
    ld.esp = env->regs[R_ESP];
    if(tb->type==TB_CALL){
        ld.type='C';
        ld.curAddr = tb->pc+tb->size-2;
    }else if(tb->type == TB_RET){
        ld.type='R';
        ld.esp -= sizeof(my_target_ulong);
        ld.curAddr = tb->pc+tb->size-1;
    }
    return ld;
}

my_target_ulong mod_core =0;
static void record_info(CPUArchState *env,CPUState *cpu,TranslationBlock *tb){
    ld = get_logdata(env,cpu,tb);
    // trim_init_extable_addr defined in MachineBit.h
    if(ld.goAddr == FUNC_PARA_MODULE){
        char mod_name[16];
        my_target_ulong mod = env->regs[R_EDI];
        cpu_memory_rw_debug(cpu,mod + moduleNameOffset,(uint8_t*)&mod_name,sizeof(mod_name),0);
        fprintf(stackWrite,MY_TARGET_FMT_lx",%s  !!!!\n",mod_core,mod_name);

        if(strcmp(mod_name,MODULE_NAME)==0){
            cpu_memory_rw_debug(cpu,mod + moduleCoreOffset,(uint8_t*)&mod_core,sizeof(mod_core),0);
            fprintf(stackWrite,MY_TARGET_FMT_lx",%s",mod_core,mod_name);
        }
    }
    

    ld_global = ld;
    int is_record_process = -1;
    is_record_process = IndexOfStr(&program_list,ld.processName);
    //only record specific function 
    if(tb->type==TB_CALL){
        switch(trace_type){
            case RECORD_SPEC_FUNC:
            {
                int funcIndex = funcistraced(ld.goAddr);
                if(funcIndex == -1) funcIndex = funcistraced(ld.goAddr - mod_core);
                if(funcIndex != -1){

                    /*   in order to record the return value, we need record current addr + 2
                     *   eg: curaddr = 0x400554, return address is 0x400556
                     *   400551:   e8 e1 ff ff ff          callq  400537 <b>
                     *   400556:   bf ff 05 40 00          mov    $0x4005ff,%edi
                    */

                    if(is_record_process !=-1){
//                        my_target_ulong retAddr = ld.curAddr+2;
                        struct ret_struct ret_tmp;
                        ret_tmp.address = ld.curAddr+2;
                        ret_tmp.ret_type = trace_func[funcIndex].ret.i_type;
                        appendList(&retAddrList,&ret_tmp); 
                        retAddrTmp = ld.curAddr+2;

                        print_log_to_file(ld);
//                        fprintf(stackWrite,TARGET_FMT_lx"-->"TARGET_FMT_lx"||",ld.curAddr,ld.goAddr);
                        fprintf(stackWrite,"%c,%s,"TARGET_FMT_lx","TARGET_FMT_lx",%d,%d,"TARGET_FMT_lx"||",ld.type,ld.processName,ld.curAddr,ld.goAddr,(int)ld.pid,ld.tid,ld.esp);
                        print_parameter(stackWrite,env,cpu,funcIndex);
                    }
                    
                }
                return ;
            }
            case RECORD_ALL_FUNC_WITHOUT_PARA:
                print_log_to_file(ld);
                return;
            case RECORD_ALL_FUNC_WITH_PARA:
                print_log_to_file(ld);
                print_all_regs_para(env);
                return;
            case RECORD_PROCESS_FUNC_WITHOUT_PARA:
                {    
                    if(is_record_process!=-1){
                        print_log_to_file(ld);
                    }
                    return;
                }
            case RECORD_PROCESS_FUNC_WITH_PARA:
                {    
                    if(is_record_process!=-1){
                        print_log_to_file(ld);
                        print_all_regs_para(env);
                    }
                    return;
                }
            case RECORD_OTHER:
                break;
            default:
                return ;
        }
    }
    else if(tb->type==TB_RET){
        switch(trace_type){
            case RECORD_SPEC_FUNC:
                {
                    if(is_record_process !=-1){
                        int ret_index = IndexOf(&retAddrList,env->eip);
                        if(ret_index != -1){
                            print_return(stackWrite,cpu,env->regs[R_EAX],ld,ret_index);
                            my_target_ulong tmp;
                            deleteList(&retAddrList,ret_index,&tmp);
                        }
                    }
                    return ;
                }
            case RECORD_ALL_FUNC_WITHOUT_PARA:
            case RECORD_ALL_FUNC_WITH_PARA:
                {
                    print_log_to_file(ld);
                    return ;
                }


            case RECORD_PROCESS_FUNC_WITHOUT_PARA:
            case RECORD_PROCESS_FUNC_WITH_PARA:
                {
                    if(is_record_process !=-1){
                        print_log_to_file(ld);
                        return ;
                    }
                }
            case RECORD_OTHER:
                break;
            default:
                return;
        }
    }


/////
    /*
    is_record_kernel = -1;
    is_record_kernel = IndexOfStr(&program_list,(char *)"kernel");
    int is_record_user = -1;
    is_record_user = IndexOfStr(&program_list,(char *)"user");
    */

    if(ld.curAddr>=kernel_addr_begin && ld.curAddr<kernel_addr_end){
        if(is_record_kernel!=-1 || is_record_process !=-1){
            print_log_to_file(ld);
            print_all_regs_para(env);
        }
    }

    if(ld.curAddr>=user_addr_begin && ld.curAddr <user_addr_end){
        if(is_record_user !=-1 ||is_record_process!=-1){
            print_log_to_file(ld);
            print_all_regs_para(env);
        }
    }

/// need record function call 
    if(print_funcstack){
        //record Stack 
        if(is_record_process!=-1){
            int inListFlag = -1;
            if(countCpuExec == 1){
                if(IndexOf(&tracePidList,ld.ppid)!=-1 || IndexOf(&tracePidList,ld.pid)!=-1){
                    inListFlag = 0; //it can be assigned to any num but -1
                }
            }
            record_stack(env,cpu,ld,inListFlag);
        }
    }
    return ;
}



/* main execution loop */

volatile sig_atomic_t exit_request;

int cpu_exec(CPUState *cpu)
{
    get_write_file();
    init_some_list();

    CPUClass *cc = CPU_GET_CLASS(cpu);
#ifdef TARGET_I386
    X86CPU *x86_cpu = X86_CPU(cpu);
    CPUArchState *env = &x86_cpu->env;
#endif
    int ret, interrupt_request;
    TranslationBlock *tb;
    uint8_t *tc_ptr;
    uintptr_t next_tb;
    SyncClocks sc;

    /* This must be volatile so it is not trashed by longjmp() */
    volatile bool have_tb_lock = false;

    if (cpu->halted) {
        if (!cpu_has_work(cpu)) {
            return EXCP_HALTED;
        }

        cpu->halted = 0;
    }

    current_cpu = cpu;

    /* As long as current_cpu is null, up to the assignment just above,
     * requests by other threads to exit the execution loop are expected to
     * be issued using the exit_request global. We must make sure that our
     * evaluation of the global value is performed past the current_cpu
     * value transition point, which requires a memory barrier as well as
     * an instruction scheduling constraint on modern architectures.  */
    smp_mb();

    rcu_read_lock();

    if (unlikely(exit_request)) {
        cpu->exit_request = 1;
    }

    cc->cpu_exec_enter(cpu);

    /* Calculate difference between guest clock and host clock.
     * This delay includes the delay of the last cycle, so
     * what we have to do is sleep until it is 0. As for the
     * advance/delay we gain here, we try to fix it next time.
     */
    init_delay_params(&sc, cpu);

    /* prepare setjmp context for exception handling */
    for(;;) {
        if (sigsetjmp(cpu->jmp_env, 0) == 0) {
            /* if an exception is pending, we execute it here */
            if (cpu->exception_index >= 0) {
                if (cpu->exception_index >= EXCP_INTERRUPT) {
                    /* exit request from the cpu execution loop */
                    ret = cpu->exception_index;
                    if (ret == EXCP_DEBUG) {
                        cpu_handle_debug_exception(cpu);
                    }
                    cpu->exception_index = -1;
                    break;
                } else {
#if defined(CONFIG_USER_ONLY)
                    /* if user mode only, we simulate a fake exception
                       which will be handled outside the cpu execution
                       loop */
#if defined(TARGET_I386)
                    cc->do_interrupt(cpu);
#endif
                    ret = cpu->exception_index;
                    cpu->exception_index = -1;
                    break;
#else
                    cc->do_interrupt(cpu);
                    cpu->exception_index = -1;
#endif
                }
            }

            next_tb = 0; /* force lookup of first TB */
            for(;;) {
                interrupt_request = cpu->interrupt_request;
                if (unlikely(interrupt_request)) {
                    if (unlikely(cpu->singlestep_enabled & SSTEP_NOIRQ)) {
                        /* Mask out external interrupts for this step. */
                        interrupt_request &= ~CPU_INTERRUPT_SSTEP_MASK;
                    }
                    if (interrupt_request & CPU_INTERRUPT_DEBUG) {
                        cpu->interrupt_request &= ~CPU_INTERRUPT_DEBUG;
                        cpu->exception_index = EXCP_DEBUG;
                        cpu_loop_exit(cpu);
                    }
                    if (interrupt_request & CPU_INTERRUPT_HALT) {
                        cpu->interrupt_request &= ~CPU_INTERRUPT_HALT;
                        cpu->halted = 1;
                        cpu->exception_index = EXCP_HLT;
                        cpu_loop_exit(cpu);
                    }
#if defined(TARGET_I386)
                    if (interrupt_request & CPU_INTERRUPT_INIT) {
                        cpu_svm_check_intercept_param(env, SVM_EXIT_INIT, 0);
                        do_cpu_init(x86_cpu);
                        cpu->exception_index = EXCP_HALTED;
                        cpu_loop_exit(cpu);
                    }
#else
                    if (interrupt_request & CPU_INTERRUPT_RESET) {
                        cpu_reset(cpu);
                    }
#endif
                    /* The target hook has 3 exit conditions:
                       False when the interrupt isn't processed,
                       True when it is, and we should restart on a new TB,
                       and via longjmp via cpu_loop_exit.  */
                    if (cc->cpu_exec_interrupt(cpu, interrupt_request)) {
                        next_tb = 0;
                    }
                    /* Don't use the cached interrupt_request value,
                       do_interrupt may have updated the EXITTB flag. */
                    if (cpu->interrupt_request & CPU_INTERRUPT_EXITTB) {
                        cpu->interrupt_request &= ~CPU_INTERRUPT_EXITTB;
                        /* ensure that no TB jump will be modified as
                           the program flow was changed */
                        next_tb = 0;
                    }
                }
                if (unlikely(cpu->exit_request)) {
                    cpu->exit_request = 0;
                    cpu->exception_index = EXCP_INTERRUPT;
                    cpu_loop_exit(cpu);
                }
                spin_lock(&tcg_ctx.tb_ctx.tb_lock);
                have_tb_lock = true;
                tb = tb_find_fast(cpu);
                /* Note: we do it here to avoid a gcc bug on Mac OS X when
                   doing it in tb_find_slow */
                if (tcg_ctx.tb_ctx.tb_invalidated_flag) {
                    /* as some TB could have been invalidated because
                       of memory exceptions while generating the code, we
                       must recompute the hash index here */
                    next_tb = 0;
                    tcg_ctx.tb_ctx.tb_invalidated_flag = 0;
                }
                if (qemu_loglevel_mask(CPU_LOG_EXEC)) {
                    qemu_log("Trace %p [" TARGET_FMT_lx "] %s\n",
                            tb->tc_ptr, tb->pc, lookup_symbol(tb->pc));
                }
                /* see if we can patch the calling TB. When the TB
                   spans two pages, we cannot safely do a direct
                   jump. */
                /*if (next_tb != 0 && tb->page_addr[1] == -1) {
                  tb_add_jump((TranslationBlock *)(next_tb & ~TB_EXIT_MASK),
                  next_tb & TB_EXIT_MASK, tb);
                  }*/
                have_tb_lock = false;
                spin_unlock(&tcg_ctx.tb_ctx.tb_lock);

                /* cpu_interrupt might be called while translating the
                   TB, but before it is linked into a potentially
                   infinite loop and becomes env->current_tb. Avoid
                   starting execution if there is a pending interrupt. */
                cpu->current_tb = tb;
                barrier();
                if (likely(!cpu->exit_request)) {
                    trace_exec_tb(tb, tb->pc);
                    tc_ptr = tb->tc_ptr;
                    /* execute the generated code */
                    next_tb = cpu_tb_exec(cpu, tc_ptr);
                    if(qemu_loglevel_mask(CPU_LOG_FUNC) && (next_tb & TB_EXIT_MASK)<2){
                        if(tb->type==TB_CALL || tb->type==TB_RET){
                            record_info(env,cpu,tb);
                            if(tb->type == TB_RET){
                                prev_is_ret = true;
                            }
                            else{
                                prev_is_ret = false;
                            }
                        }
                        /*
                        else if(prev_is_ret && strcmp(ld_global.processName,"mychmod")==0){
                            my_target_ulong retAddr = tb->pc+tb->size-2;
                            fprintf(stackWrite,MY_TARGET_FMT_lx","MY_TARGET_FMT_lx"\n",retAddrTmp,retAddr);
                            //if(IndexOf(&retAddrList,retAddr)!=-1){
                            //if(retAddrTmp-retAddr<=5 && retAddrTmp-retAddr>=-5){
                            if(retAddr==0xc10efdff || retAddr==0xc162fa4c){
                                print_return(stackWrite,env->regs[R_EAX],retAddr,ld_global);
                            }
                        }
                        */
                    }                    

                    switch (next_tb & TB_EXIT_MASK) {
                        case TB_EXIT_REQUESTED:
                            /* Something asked us to stop executing
                             * chained TBs; just continue round the main
                             * loop. Whatever requested the exit will also
                             * have set something else (eg exit_request or
                             * interrupt_request) which we will handle
                             * next time around the loop.
                             */
                            next_tb = 0;
                            break;
                        case TB_EXIT_ICOUNT_EXPIRED:
                            {
                                /* Instruction counter expired.  */
                                int insns_left = cpu->icount_decr.u32;
                                if (cpu->icount_extra && insns_left >= 0) {
                                    /* Refill decrementer and continue execution.  */
                                    cpu->icount_extra += insns_left;
                                    insns_left = MIN(0xffff, cpu->icount_extra);
                                    cpu->icount_extra -= insns_left;
                                    cpu->icount_decr.u16.low = insns_left;
                                } else {
                                    if (insns_left > 0) {
                                        /* Execute remaining instructions.  */
                                        tb = (TranslationBlock *)(next_tb & ~TB_EXIT_MASK);
                                        cpu_exec_nocache(cpu, insns_left, tb);
                                        align_clocks(&sc, cpu);
                                    }
                                    cpu->exception_index = EXCP_INTERRUPT;
                                    next_tb = 0;
                                    cpu_loop_exit(cpu);
                                }
                                break;
                            }
                        default:
                            break;
                    }
                }
                cpu->current_tb = NULL;
                /* Try to align the host and virtual clocks
                   if the guest is in advance */
                align_clocks(&sc, cpu);
                /* reset soft MMU for next block (it can currently
                   only be set by a memory fault) */
            } /* for(;;) */
        } else {
            /* Reload env after longjmp - the compiler may have smashed all
             * local variables as longjmp is marked 'noreturn'. */
            cpu = current_cpu;
            cc = CPU_GET_CLASS(cpu);
            cpu->can_do_io = 1;
#ifdef TARGET_I386
            x86_cpu = X86_CPU(cpu);
            env = &x86_cpu->env;
#endif
            if (have_tb_lock) {
                spin_unlock(&tcg_ctx.tb_ctx.tb_lock);
                have_tb_lock = false;
            }
        }
    } /* for(;;) */

    cc->cpu_exec_exit(cpu);
    rcu_read_unlock();

    /* fail safe : never use current_cpu outside cpu_exec() */
    current_cpu = NULL;
    return ret;
}
