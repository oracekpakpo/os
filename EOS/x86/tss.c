#include <x86/tss.h>

#define tss_access 0xe9
#define tss_flag 0x00

sys_tss_t tss;
extern sys_gdt_t gdt[];

void  ltss()
{
    __asm__ __volatile("movw %0, %%ax;"
                        "ltr %%ax;"
                        ::"i"(0x2b));
}

void init_tss()
{
    kprintf("Installing TSS...");
    u32 base =(u32)&tss;
    u32 limit= base + sizeof(sys_tss_t);
    init_gdt_des(base, limit ,tss_access,tss_flag, &gdt[5]); // tss entry in gdt 0x28 | 0x03 = 0x2b
    ltss();
    kprintf("[ok]\n");
}

