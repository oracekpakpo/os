#ifndef __ASM__IO__H__
#define __ASM__IO__H__

#include <types.h>

static inline void cli()
{
    __asm__ __volatile__("cli");
}

static inline void sti()
{
    __asm__ __volatile__("sti");
}

static inline void outb(u16 port, u8 value)
{
    __asm__ __volatile__("outb %%al, %%dx;"
            ::"a"(value),"d"(port));
}

static inline void outbp(u16 port, u8 value)
{
    __asm__ __volatile__("outb %%al, %%dx; jmp 1f;1:"
            ::"a"(value),"d"(port));
}


static inline u8 inb(u16 port)
{
    u8 value;
    __asm__ __volatile__("inb %%dx, %%al":"=a"(value):"d"(port));
    return value;
}

static inline void outl(u16 port, u32 data)
{
    __asm__ __volatile__("outl %%eax, %%dx"
                        ::"a"(data),"d"(port));
}

static inline u32 inl(u16 port)
{
    u32 value;

    __asm__ __volatile__("inl %%dx, %%eax"
                        :"=a"(value):"d"(port));
                        return value;
}

static inline void outw(u16 port, u16 data)
{
    __asm__ __volatile__("outw %%ax, %%dx"
                        ::"a"(data),"d"(port));
}

static inline u16 inw(u16 port)
{
    u16 value;
    __asm__ __volatile__("inw %%dx, %%ax"
                        :"=a"(value):"d"(port));
                        return value;
}

#endif
