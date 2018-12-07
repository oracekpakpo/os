#include <x86/gdt.h>
#include <lib.h>

sys_gdt_t gdt[gdt_size]; // system gdt table declaration
sys_gdtr_t gdtr; // system gdt register


void init_gdt_des(u32 base, u32 limit, uchar access, uchar flag, sys_gdt_t* gdt_des)
{
    gdt_des->limit_0_15=(u16)(limit & 0x0000ffff);
    gdt_des->base_16_31= (u16)(base & 0x0000ffff);
    gdt_des->base_32_39 =((u8)( base >> 16) & 0x000000ff);
    gdt_des->access=access;
    gdt_des->limit_48_51=((limit >> 16) & 0x0000000f);
    gdt_des->flags=(flag & 0x0f);
    gdt_des->base_56_63=((base >> 24) & 0x000000ff );
}

void lgdtr()
{
    gdtr.limit=sizeof(sys_gdt_t)*gdt_size -1;
    gdtr.base=(u32 )(&gdt);
    __asm__ __volatile__("lgdt %0"::"m"(gdtr));
}

void init_gdt()
{
    kprintf("Installing GDT... ");
    init_gdt_des(0x0,0x0,0x0,0x0,&gdt[0]); // null descriptor
    init_gdt_des(0x0,0xfffff,DESCRIPTOR_PRESENT|DESCRIPTOR_DPL0|SEGMENT_DESCRIPTOR|NO_CONFORMING_DESCRIPTOR|READABLE_DESCRIPTOR,
    FLAG,&gdt[1]); // kernel code descriptor
    init_gdt_des(0x0,0xfffff,DESCRIPTOR_PRESENT|DESCRIPTOR_DPL0|SEGMENT_DESCRIPTOR|NO_EXPANDING_DESCRIPTOR|WRITABLE_DESCRIPTOR,
    FLAG,&gdt[2]); // kernel data descriptor
    init_gdt_des(0x0,0xfffff,DESCRIPTOR_PRESENT|DESCRIPTOR_DPL3|SEGMENT_DESCRIPTOR|CONFORMING_DESCRIPTOR|READABLE_DESCRIPTOR,
    FLAG, &gdt[3]); // user code descriptor
    init_gdt_des(0x0,0xfffff,DESCRIPTOR_PRESENT|DESCRIPTOR_DPL3|SEGMENT_DESCRIPTOR|CONFORMING_DESCRIPTOR|WRITABLE_DESCRIPTOR,
    FLAG,&gdt[4]); // user data descriptor
    memset(&gdt[5],0,sizeof(sys_gdt_t));
    lgdtr();
    kprintf("[ok]\n");
}
