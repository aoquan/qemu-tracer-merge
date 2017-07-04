#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/kprobes.h>
#include <linux/dcache.h>
#include <linux/fs.h>

#define PROGRAM_NAME "overlayfail"
static char func_name[NAME_MAX] = "ovl_setattr"; // 跟踪对象
//static char func_name[NAME_MAX] = "ext4_setattr"; // 跟踪对象

static struct kretprobe copy_process_kret = {
	.kp.symbol_name		= func_name,
};

static int ret_handler(struct kretprobe_instance *ri, struct pt_regs *regs)
{
	if(!strcmp(current->comm, PROGRAM_NAME)) {
	 printk(KERN_INFO "return value: %d\n", regs->ax);
	}

	return 0;
}

static struct kprobe copy_process_kp = {
	.symbol_name	= func_name,
};

static int pre_process_handler(struct kprobe *p, struct pt_regs *regs)
{
	if(!strcmp(current->comm, PROGRAM_NAME))
	{
	struct iattr * iat = (struct iattr *) (regs->si);
	printk(KERN_INFO "hello world\n");
	printk(KERN_INFO "iattr,mode:%o, uid:%d, gid:%d\n",iat->ia_mode,iat->ia_uid,iat->ia_gid);
	
	struct dentry * dey = (struct dentry *)(regs->di);
	struct inode * ind = dey->d_inode;
	struct qstr qr = dey->d_name;
	printk(KERN_INFO "dentry inode,mode:%o, uid:%d, gid:%d, name:%s\n",ind->i_mode,ind->i_uid,ind->i_gid,qr.name);
        printk(KERN_INFO "task_stuct: funcname: %s, pid: %d, comm: %s, uid: %d, gid: %d\n",func_name,current->pid, current->comm,current->cred->uid, current->cred->gid);
	}
	return 0;
}

static int __init kretprobe_init(void)
{
	int kp_ret, kret_ret;

	copy_process_kp.pre_handler = pre_process_handler;
	kp_ret = register_kprobe(&copy_process_kp);
	if (kp_ret < 0)
	{
		printk(KERN_INFO "register_kprobe of copy_process_kp failed, returned %d\n", kp_ret);
	}
	else
	{
		printk(KERN_INFO "Planted copy_process_kp at %p\n", copy_process_kp.addr);
	}

	copy_process_kret.handler = ret_handler;
	kret_ret = register_kretprobe(&copy_process_kret);
	if (kret_ret < 0)
	{
		printk(KERN_INFO "register_kretprobe of copy_process_kret failed, returned %d\n", kret_ret);
		return -1;
	}
	printk(KERN_INFO "Planted copy_process_kret at %p\n", copy_process_kret.kp.addr);
	return 0;
}

static void __exit kretprobe_exit(void)
{
	unregister_kretprobe(&copy_process_kret);
	printk(KERN_INFO "copy_process_kret at %p unregistered\n", copy_process_kret.kp.addr);

	unregister_kprobe(&copy_process_kp);
	printk(KERN_INFO "copy_process_kp at %p unregistered\n", copy_process_kp.addr);
}

module_init(kretprobe_init)
module_exit(kretprobe_exit)
MODULE_LICENSE("GPL");
