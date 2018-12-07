#ifndef __GDT_H__
#define __GDT_H__

#include <types.h>

#define KERNEL_CS 0x08
#define KERNE_DS 0x10
#define KERNEL_ES 0x10
#define KERNEL_FS 0x10
#define KERNEL_GS 0x10
#define KERNEL_SS 0x10

#define USER_CS 0x1b
#define USER_DS 0x23
#define USER_ES 0x23
#define USER_FS 0x23
#define USER_GS 0x23
#define USER_SS 0x23
#define gdt_size 6

#define DESCRIPTOR_FLAG 0x0c
#define DESCRIPTOR_PRESENT 0x80
#define DESCRIPTOR_DPL0 0x00
#define DESCRIPTOR_DPL1 0x20
#define DESCRIPTOR_DPL2 0x40
#define DESCRIPTOR_DPL3 0x60
#define SEGMENT_DESCRIPTOR 0x10
#define SYSTEM_DESCRIPTOR 0x00
#define CONFORMING_DESCRIPTOR 0x0c
#define NO_CONFORMING_DESCRIPTOR 0x08
#define READABLE_DESCRIPTOR 0x02
#define EXPANDING_DESCRIPTOR 0x04
#define NO_EXPANDING_DESCRIPTOR 0x00
#define WRITABLE_DESCRIPTOR 0x02

typedef struct sys_gdt
{
    u16 limit_0_15;
    u16 base_16_31;
    uchar base_32_39;
    uchar access;
    uchar limit_48_51:4;
    uchar flags:4;
    uchar base_56_63;
}__attribute__((packed)) sys_gdt_t;


typedef struct sys_gdtr
{
    u16 limit;
    u32 base;
}__attribute__((packed)) sys_gdtr_t ;


void init_gdt();
void init_gdt_des(u32 base, u32 limit, uchar access, uchar flag, sys_gdt_t* gdt_des);

#endif
