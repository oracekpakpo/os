#include <x86/idt.h>

#define PIT_0 0x40
#define PIT_CMD_REG 0x43
#define channel_0 0x36
#define divisor 47727 // get IRQ_0 each 40ms

extern void isr_0();
extern void isr_1();
extern void isr_2();
extern void isr_3();
extern void isr_4();
extern void isr_5();
extern void isr_6();
extern void isr_7();
extern void isr_default();

struct sys_idt idt[idt_size]; // System interrupt vector table
struct sys_idtr idtr; //

void load_idtr()
{
    cli();
    idtr.base=(u32)&idt;
    idtr.limit=idt_size*8 -1;
    __asm__ __volatile__("lidt %0"::"m"(idtr));
    sti();
}

void init_interrupt_des(u16 segment, u32 offset,struct sys_idt* sys_int)
{
    sys_int->segment=segment;
    sys_int->offset0_15=(u16)(offset & 0x0000ffff);
    sys_int->offset16_31=(u16)((offset >> 16) & 0x0000ffff);
    sys_int->reserved=0x0;
    sys_int->flags=intgate;
}

void init_trap_des(u16 segment, u32 offset, struct sys_idt* sys_trap)
{
    sys_trap->segment=segment;
    sys_trap->offset0_15=(u16)(offset & 0x0000ffff);
    sys_trap->offset16_31=(u16)((offset >> 16) & 0x0000ffff);
    sys_trap->reserved=0x0;
    sys_trap->flags=trapgate;
}

void set_int_handler(u32 handler, struct sys_idt * sys_int)
{
    sys_int->offset0_15=(u16)(handler & 0x0000ffff);
    sys_int->offset16_31=(u16)((handler >> 16 )& 0x0000ffff);
    sys_int->flags=trapgate;
}

void SET_INTERRUPT(u32 handler, u8 num)
{
    set_int_handler(handler,&idt[num]);
}

void set_irq_handler(u32 handler, u8 irq_num)
{

    if(irq_num<0 || irq_num >18)
    {
        perror("Invalid irq number.\n");
        return;
    }

    if(irq_num<8)
    {
        idt[0x20+irq_num].offset0_15=(u16)(handler & 0x0000ffff);
        idt[0x20+irq_num].offset16_31=(u16)((handler>>16) & 0x0000ffff);
        idt[0x20+irq_num].flags=intgate;
    }
    else
    {
        irq_num-=8;
        idt[0x70+irq_num].offset0_15=(u16)(handler & 0x0000ffff);
        idt[0x70+irq_num].offset16_31=(u16)((handler>>16) & 0x0000ffff);
        idt[0x70+irq_num].flags=intgate;
    }
}

void init_pic8259() // PIC 8259 configuration
{
// ICW4 , cascade niveau de priorite
    outbp(0x20,0x19);
    outbp(0xa0,0x19);

// Adresse de base des vecteurs d'interuuption
    outbp(0x21,0x20);
    outbp(0xa1,0x70);

    outbp(0x21,0x04);
    outbp(0xa1,0x02);

    outbp(0x21,0x01);
    outbp(0xa1,0x01);

}

void init_idt_interrupt()
{
    for(int i=0;i<idt_size;i++)
        init_interrupt_des(0x08,(u32)(isr_default),&idt[i]);
    
    kprintf("Installing system interrupt... ");
    set_irq_handler((u32)isr_0,0);
    set_irq_handler((u32)isr_1,1);
    set_irq_handler((u32)isr_2,2);
    set_irq_handler((u32)isr_3,3);
    set_irq_handler((u32)isr_4,4);
    set_irq_handler((u32)isr_5,5);
    set_irq_handler((u32)isr_6,6);
    set_irq_handler((u32)isr_7,7);    
    kprintf("[ok]\n");
}



void pit_config()
{
    outb(PIT_CMD_REG,channel_0);
    outb(PIT_0,(u8)(divisor & 0x00ff));
    outb(PIT_0,(u8)(divisor >> 8));
}// assign activity quantum to 60 ms

void init_idt()
{
    init_pic8259();
    init_idt_interrupt();
    load_idtr();
    pit_config();
}

void mask_irq(u8 irq)
{
    u8 state=inb(IMR);
    state |= irq;
    outb(IMR,state);
}

void enable_irq()
{
    outb(IMR,0x00);
}
