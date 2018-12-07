#ifndef __IDT_H__
#define __IDT_H__

#include <types.h>
#include <io.h>

#define idt_size 256
#define intgate 0x8e
#define trapgate 0xef
#define IMR 0x21
#define IRQ_0 0x01
#define IRQ_1 0x02
#define IRQ_2 0x04
#define IQR_3 0x08
#define IRQ_4 0x10
#define IRQ_5 0x20
#define IRQ_6 0x40
#define IRQ_7 0x80


struct sys_idt
{
    u16 offset0_15;
    u16 segment;
    u8 reserved;
    u8 flags;
    u16 offset16_31;
}__attribute((packed));

struct sys_idtr
{
    u16 limit;
    u32 base;
}__attribute__((packed));


void mask_irq(u8 irq);
void enable_irq();
void init_interrupt_des(u16 segment, u32 offset,struct sys_idt* sys_int);
void init_trap_des(u16 segment, u32 offset, struct sys_idt* sys_trap);
void load_idtr();
void set_int_handler(u32 handler, struct sys_idt* sys_int);
void set_irq_handler(u32 handler, u8 irq_num);
void SET_INTERRUPT(u32 handler, u8 num);
void init_idt();
void init_idt_interrupt();
void init_pic8259();


#endif
