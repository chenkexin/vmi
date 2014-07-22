#include<linux/init.h> 
#include<linux/module.h> 
#include <linux/unistd.h>
#define USE_IMMEDIATE

MODULE_LICENSE("Dual BSD/GPL"); 
/*

1. get systemcall addr: __NR_open
2. set the __NR_open addr to %%dr0
3. set %%dr6 to single step
4. set INT 0x01 handler to my_hook
5. use a user process to call "open"

*/
struct descriptor_idt
{
        unsigned short offset_low,seg_selector;
        unsigned char reserved,flag;
        unsigned short offset_high;
};

static void __check_register(void)
{
    //this function will print dr0 dr6 and dr7
    int dr0;
    int dr6;
    int dr7;

    /*clobber register is no need specified here because =a =b =c has specified these three. */ 
   __asm__ volatile("movl %%dr0,%%eax \n\t"
                        "movl %%dr6, %%ebx \n\t"
                        "movl %%dr7,%%ecx \n\t"
                        : "=a" (dr0), "=b" (dr6), "=c" (dr7)
                        :
                        : 
                        );
    printk(KERN_INFO "in checking_register: DR0-%08x, DR6-%08x, DR7-%08x\n", dr0, dr6, dr7);
}

static void __set_dr0(int syscall)
{
    printk(KERN_INFO "in __set_dr0: %08x", syscall);
    //this function will set dr0 to a function call's value
    __asm__ volatile("mov %0,%%dr0     \n\t"
                        :
                        :"r" (syscall)
                        :
                        );
    __check_register(); 
}

static int __get_int_handler(int idt_number)
{
    int idt_entry = 0;
    __asm__ volatile( "xorl %%ebx, %%ebx    \n\t"
                        "pushl %%ebx            \n\t"
                        "pushl %%ebx            \n\t"
                        "sidt (%%esp)           \n\t"
                        "movl 2(%%esp), %%ebx   \n\t"
                        "movl %1, %%ecx         \n\t"
                        "leal (%%ebx,%%ecx,8),%%esi \n\t"
                        "xorl %%eax, %%eax      \n\t"
                        "movw 6(%%esi), %%ax    \n\t"
                        "roll $0x10, %%eax      \n\t"
                        "movw (%%esi),%%ax      \n\t"
                        "popl %%ebx             \n\t"
                        "popl %%ebx             \n\t"
                        : "=a" (idt_entry)
                        : "r" (idt_number)
                        : "ebx", "esi");
    return idt_entry;
}

static int __get_func_addr(int syscall)
{
    //this function return syscall's addr according to its syscall number
   /*
    1. get INT 0x80 handler 
    2. get syscall_call
    3. get sys_call_table addr using syscall_call(compare the first several bytes
    4. find the syscall addr using the table and syscall number
    */ 
    unsigned int table;
    
    int syscall_call = __get_int_handler(0x80);
    
    unsigned char *p = (unsigned char*)syscall_call;
    #if defined(__x86_64__)
    #pragma once
    while(!((p[0] == 0xff) &&(p[1] == 0x14) && (p[2] == 0xc5)))
    {
        p++;
    }
    //assume else is x86 
    #else
    while(!((p[0] == 0xff) &&(p[1] == 0x14) && (p[2] == 0x85)))
    {
        p++;
    } 
    #endif
    /*printk(KERN_INFO " checking TABLE! %08x\n", p[0]);
    printk(KERN_INFO " checking TABLE! %08x\n", p[1]);
    printk(KERN_INFO " checking TABLE! %08x\n", p[2]);
    printk(KERN_INFO " checking TABLE! %08x\n", p);*/

    table = *(unsigned int *)(p+3);
    //extract func addr according to syscall_table
    void **syscall_table = (void**)table; 
    return syscall_table[syscall]; 
}
static unsigned long ptr_idt_table;

static void* get_stub_from_idt (int n)
{
    printk(KERN_INFO "check idt addr:%08x\n", ptr_idt_table);    
    printk(KERN_INFO "descriptor size: %d\n", sizeof(struct descriptor_idt));
        struct descriptor_idt *idt = &(((struct descriptor_idt *)ptr_idt_table) [n]);
    printk(KERN_INFO " idt_entry addr: %08x\n", idt);
        unsigned long addr= (unsigned long) ((void *) ((idt->offset_high << 16 ) + idt->offset_low));
        printk(KERN_INFO "in get_stub_from_idt, do_debug addr: %08x", addr);
        return ((void*) ((idt->offset_high<<16) + idt->offset_low));
}

static void my_hook()
{
    printk(KERN_INFO "already in hooking!\n");
}

static void hook_stub(int n,void *new_stub,unsigned long *old_stub)
{
         unsigned long new_addr=(unsigned long)new_stub;
         printk(KERN_INFO "1\n");
         struct descriptor_idt *idt=(struct descriptor_idt *)ptr_idt_table;
         //save old stub

         if(old_stub)
         {
                printk(KERN_INFO "2\n");
                *old_stub=(unsigned long)get_stub_from_idt(1);
                printk(KERN_INFO "in hook_stub, INT0x01 handler:%08x\n", *old_stub);
         }
        //assign new stub
         idt[n].offset_high = (unsigned short) (new_addr >> 16);
         idt[n].offset_low  = (unsigned short) (new_addr & 0x0000FFFF);
         printk(KERN_INFO "finish hook\n");
         return;
}

static unsigned long get_addr_idt (void)
{
    unsigned char idtr[6];
    unsigned long idt;
    __asm__ volatile ("sidt %0": "=m" (idtr));
    idt = *((unsigned long *) &idtr[2]);
    printk(KERN_INFO "in get_addr_idt, idt addr: %08x", idt);
    return(idt);
}


static int __set_debug_control(void)
{
    //this function will set correct debug control: dr6 and dr7
    //generate 0x01 exception
    
    /* 
        dr6: BS(6th bit) = 1: single step trap
    */ 
    int bs = 0;
    int dr7= 0;
    printk(KERN_INFO "before set DBG\n");
    __check_register();
    __asm__ volatile("mov %%dr6, %0  \n\t"
                    :"=a"(bs)
                    :
                    :);
    bs |= (0<<6);
    bs |= (1);
    __asm__ volatile("mov %0, %%dr6   \n\t"
                    :
                    :"r"(bs)
                    :);
    printk(KERN_INFO " after set DBG\n");
    __check_register();
    //set dr7
   /* 
    __asm__ volatile("mov %%dr7, %0  \n\t"
                    :"=a"(dr7)
                    :
                    :);
    dr7 |= (1);
    __asm__ volatile("mov %0, %%dr7   \n\t"
                    :
                    :"r"(dr7)
                    :);
    __check_register();*/
} 
  
static __init int hello_init(void) 
{
int h0x80;
int func_addr;
unsigned long a = 0;
 
printk(KERN_ALERT"HELLO WORLD\n"); 

ptr_idt_table=get_addr_idt();
__check_register();

//getting int 80 handler, how to check?
//h0x80 = __get_int_handler(0x80);
//printk(KERN_INFO " handler 0x80 : %08x ", h0x80);

func_addr = __get_func_addr(__NR_open);
__set_dr0(func_addr);

__set_debug_control();

hook_stub(1, &my_hook, &a);

return 0; 
}
 
static __exit void hello_exit(void) 
{
int a=0;
__set_dr0(a); 
printk(KERN_ALERT"GoodBye\n"); 
} 
module_init(hello_init); 
module_exit(hello_exit); 