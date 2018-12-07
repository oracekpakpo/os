#ifndef _TSS_H__
#define _TSS_H__

#include <types.h>
#include <lib.h>
#include <x86/gdt.h>

typedef struct sys_tss
{
    u16 link, unused_link;
    u32 esp0;
    u16 ss0, unused_ss0;
    u32 esp1;
    u16 ss1,unused_ss1;
    u32 esp2;
    u16 ss2, unused_ss2;
    u32 cr3;
    u32 eip;
    u32 eflags;
    u32 eax;
    u32 ecx;
    u32 edx;
    u32 ebx;
    u32 esp;
    u32 ebp;
    u32 esi;
    u32 edi;
    u16 es, unused_es;
    u16 cs,unused_cs;
    u16 ss ,unused_ss;
    u16 ds,unused_ds;
    u16 fs,unused_fs;
    u16 gs, unused_gs;
    u16 ldtr, unused_ldtr;
    u16 unused_iomap, iomap;
} __attribute__((packed)) sys_tss_t;

void init_tss();
void ltss();
#endif
