
     static int x=0;
     static int y=0;
//struct Cursor cursor;


//#include "keyboard_driver.h"


#define keyboard_cmd_reg 0x64
#define keyboard_data_reg 0x60
#ifndef AZERTY
#define AZERTY
#include "scancode.h"
#endif // AZERTY


void init_keyboard(void);

typedef unsigned char * memory;
typedef unsigned char uchar;
typedef unsigned int u32;
typedef unsigned char u8;
typedef unsigned short u16;

uchar azerty_keyboard(uchar key)
{
    switch(key)
    {

        case akey_a:
        return 'a';
        case akey_z:
        return 'z';
        case akey_e:
        return 'e';
        case akey_r:
        return 'r';
        case akey_t:
        return 't';
        case akey_y:
        return 'y';
        case akey_u:
        return 'u';
        case akey_i:
        return 'i';
        case akey_o:
        return 'o';
        case akey_p:
        return 'p';
        case akey_q:
        return 'q';
        case akey_s:
        return 's';
        case akey_d:
        return 'd';
        case akey_f:
        return 'f';
        case akey_g:
        return 'g';
        case akey_h:
        return 'h';
        case akey_j:
        return 'j';
        case akey_k:
        return 'k';
        case akey_l:
        return 'l';
        case akey_m:
        return 'm';
        case akey_w:
        return 'w';
        case akey_x:
        return 'x';
        case akey_c:
        return 'c';
        case akey_v:
        return 'v';
        case akey_b:
        return 'b';
        case akey_n:
        return 'n';
        case akey_enter:
        return '\n';
        case akey_Tab:
        return '\t';
        case akey_space:
        return ' ';
        case akey_num_0:
        return '0';
        case akey_num_1:
        return '1';
        case akey_num_2:
        return '2';
        case akey_num_3:
        return '3';
        case akey_num_4:
        return '4';
        case akey_num_5:
        return '5';
        case akey_num_6:
        return '6';
        case akey_num_7:
        return '7';
        case akey_num_8:
        return '8';
        case akey_num_9:
        return '9';
        case akey_mul:
        return '*';
        case akey_num_minus:
        return '-';
        case akey_num_plus:
        return '+';
//        case akey_slash:
//        return '/';
        case akey_0_adresse:
        return '@';
        case akey_1_et:
        return '&';
        case akey_2point_slash:
        return ':';
        case akey_2_et_tilde:
        return '~';
        case akey_3_2quote_diese:
        return '"';
        case akey_4_quote_oaccolade:
        return '\'';
        case akey_5_oparenthese_ocrochet:
        return '(';
        case akey_6_moins_bar:
        return '-';
        case akey_7:
        return '`';
        case akey_8_underscore_antislash:
        return '_';
        case akey_9:
        return '^';
        case akey_equal_plus_faccolade:
        return '=';
        case akey_fparenthese_fcrochet:
        return ')';
        case akey_dollar:
        return '$';
        case akey_chapeau:
        return '^';
        case akey_chevron:
        return '<';
        case akey_virgule_interrogation:
        return ',';
        case akey_pvirgule_point:
        return ';';
        case akey_exclamation:
        return '!';
        case akey_percente_u:
        return '%';
        case akey_star:
        return '*';
        default:
        return 0;
    }

}


void outb(u16 port, u8 value)
{
    __asm__ __volatile__("outb %%al, %%dx;"
            ::"a"(value),"d"(port));
}

void outw(u16 port, u16 value)
{
    __asm__ __volatile__("outw %%ax, %%dx;"
            ::"a"(value),"d"(port));
}

void outbp(u16 port, u8 value)
{
    __asm__ __volatile__("outb %%al, %%dx; jmp 1f;1:"
            ::"a"(value),"d"(port));
}


u8 inb(u16 port)
{

u8 value;
__asm__ __volatile__("inb %%dx, %%al":"=a"(value):"d"(port));
return value;
}
 u16 inw(u16 port)
 {
    u16 value;
    __asm__ __volatile__("inw %%dx, %%ax":"=a"(value):"d"(port));
    return value;
 }


uchar read_cmd_reg()
{
    uchar data;
    data=inb(keyboard_cmd_reg);
    return data;
}

void write_cmd_reg(uchar data)
{
    outb(keyboard_cmd_reg,data);
}


uchar read_data_reg()
{
    uchar data;
    data=inb(keyboard_data_reg);
    return data;
}

void write_data_reg(uchar data)
{
    outb(keyboard_data_reg,data);
}

void move_cursor(u8 x ,u8 y)
{
    u16 c_pos;
    c_pos= y*80 + x;

    outb(0x3d4,0x0f);
    outb(0x3d5,(u8) c_pos);
    outb(0x3d4,0x0e);
    outb(0x3d5,(u8) (c_pos>>8));
}

void show_cursor()
{
    move_cursor(x,y);
}

void scrollup(u32 n)
{

volatile memory video;
volatile memory tmp;
memory i;

for(i=(memory)0xB8000;i<(memory)0xB8FA0;i+=2)
{
    tmp =(volatile memory)(i+n*160);
    video=(volatile memory)i;

    if(tmp <(memory)0xB8FA0)
    {
    *video= *tmp;
    *(video+1)=*(tmp+1);
    }else
    {
    *video=0;
    *(video+1)=0x07;
    }

}

y-=n;
if(y<0) y=0;
}
void write_char(uchar c, u32 color)
{

      if(c==0)
      {
      show_cursor();
        return;
      }
    if(c==10)
      {
        x=0;
        y++;
      }else
      if(c==9)
      {
        x=x+4;
      }else
      if(c==13)
      {
        x=0;
      }
      else
      {

        volatile memory video = (volatile memory)(0xB8000+2*x+160*y);
      *video=c;
      *(video+1)=color;
      x ++;

      if(x>79)
      {
        x=0;
        y++;
      }

      if(y>24)
      {
        scrollup(y-24);
      }
        show_cursor();
        }
}

extern  void function();

void write_string(const uchar* string, u32 color)
{
    volatile char* video=(volatile char*) 0xB8000;

    while(*string !=0)
    {
        uchar c=*string;
      if(c==10)
      {
        x=0;
        y++;
      }else
      if(c==9)
      {
        x=x+8;
      }else
      if(c==13)
      {
        x=0;
      }else
	{
      video = (volatile char*)(0xB8000 +2*x+160*y);
      *video=c;
      *(video+1)=color;
      x ++;

      if(x>79)
      {
       	x=0;
        y++;
      }
}

      if(y>24)
      {
      scrollup(y-24);
      }

	show_cursor();
      string++;
    }

}


struct idtdesc
{

u16 offset0_15;
u16 select;
u16 type;
u16 offset16_31;
}__attribute__((packed));


void init_idt_desc(u16 select,u32 offset,u16 type,struct idtdesc* desc)
{

desc->offset0_15=(offset &0xffff);
desc->select=select;
desc->type=type;
desc->offset16_31=(offset & 0xffff0000)>>16;

}
void sys_printf(uchar*);
void print_hex(u32);



extern void _switch();
void switch_to(u16 cs, u32 eip)
{
    __asm__ __volatile__("push %%ss;push %%esp;"
                        "pushf;"
                        "jmp next;"
                        "next:\n"
                        "push %0;push %1;"
                        "ljmp $0x08, $_switch;"
                        ::"m"(cs),"m"(eip));
}
void init_icw();
void task1();
void task();
void cli();
void sti();

void task()
{
sti();
init_icw();
int a=100;
int b=87;
print_hex(a);
sys_printf("+");
print_hex(b);
sys_printf("=");
print_hex(a+b);
sys_printf("\n");

//switch_to(0x08,(u32)task1);
//switch_to(0x08,(u32)task1);
while(1);
}

void task1()
{
    sys_printf("Into task...\n");

    int i=0;
for(i=145;i<=150;i++)
{
    print_hex(i); sys_printf("\n");
}

switch_to(0x08,(u32)task);
      while(1);
}

void irq0()
{
     static int i=0;
     i++;
     if((i%20)==0)
     {
         //switch_to(0x08,(u32)task1);
     }


};

void irq1(void)
{

//__asm__ __volatile__("cli");
char c;
do
{
c=inb(0x64);
}while((c&0x01)==0);

c=inb(0x60);

if(c<0x80)
{
    char e=azerty_keyboard(c);
    write_char(e,0x02);
}
else
{}

}

void defaut_irq()
{
write_string("Default\n",0x02);
}

void sys_printf(uchar* msg)
{
    write_string(msg,0x07);
}
extern void asm_irq_0();
extern void asm_irq_1();
extern void default_irq();
extern void init_icw();
extern void sys_call();
#define IDTSIZE 256
#define intgate 0b1000111000000000
#define trapgate 0xef00
struct idtdesc idt[IDTSIZE];

struct idtr
{
u16 limit;
u32 base;
} __attribute__((packed));


void init_idt()
{

int i;
for(i=0;i<IDTSIZE;i++)
{
init_idt_desc(0x08,(u32)default_irq,intgate,&idt[i]);
}

init_idt_desc(0x08,(u32)asm_irq_0,intgate,&idt[32]);
init_idt_desc(0x08,(u32)asm_irq_1,intgate,&idt[33]);
init_idt_desc(0x08,(u32)sys_call,trapgate,&idt[0x30]);
}

void cli()
{
    __asm__ __volatile__("cli"::);
}


void sti()
{
    __asm__ __volatile__("sti"::);
}


void kbconfig()
{
outb(0x64,0xf9);
}



typedef int size_t;

void *memcpy(void *dest, void *src, size_t n)
{
	size_t i;

	for(i = 0; i < n; i++)
	{
		((uchar*)dest)[i] = ((uchar*)src)[i];
	}

	return dest;
}



#define mask 0x0000000f

void print_hex(u32 n)
{
    char* format ="0x00000000";
    char* key="0123456789ABCDEF";

    u32 tampon;
    tampon = n & mask;
    format[9]=key[tampon];

    tampon= (n>>4) & mask;
    format[8]=key[tampon];

    tampon= (n>>8) & mask;
    format[7]=key[tampon];

    tampon= (n>>12) & mask;
    format[6]=key[tampon];

    tampon= (n>>16) & mask;
    format[5]=key[tampon];

    tampon= (n>>20) & mask;
    format[4]=key[tampon];

    tampon= (n>>24) & mask;
    format[3]=key[tampon];

    tampon= (n>>28) & mask;
    format[2]=key[tampon];

    write_string(format,0x02);
}

static u32 page_directory[1024] __attribute__((aligned(4096)));
u32* page_table=(u32*)0x400000;
u32 var=0x400000;
void init_page()
{
int i=0;
page_table=(u32*)0x400000;
    for(i=0;i<1024*1024;i++)
    {
        page_table[i]=((i*0x1000)|3);

    }

}

void init_dir()
{
int i=0;
for(i=0;i<1024;i++)
    page_directory[i]= (u32)(page_table + i*1024)|3;
}

void bl_common(int drive, int numblock,int count)
{
    outb(0x1f1,00);
    outb(0x1f2,count);
    outb(0x1f3,(uchar)numblock);
    outb(0x1f4,(uchar)(numblock>>8));
    outb(0x1f5,(uchar)(numblock>>16));

    outb(0x1f6,0xe0|(drive<<4)|(numblock>>24)&0x0f);

}

int bl_read(int drive, int numblock, int count , char* buf)
{
    u16 tmpword;
    int idx;
    bl_common(drive,numblock,count);
    outb(0x1f7,0x20);
    while(!(inb(0x1f7)&0x08));

    for(idx=0;idx<256*count;idx++)
    {
        tmpword=inw(0x1f0);
        buf[idx*2]=(uchar)tmpword;
        buf[idx*2+1]=(uchar)(tmpword>>8);
    }

    return count;
}


int bl_write(int drive, int numblock, int count , char* buf)
{
    u16 tmpword;
    int idx;
    bl_common(drive,numblock,count);
    outb(0x1f7,0x30);
    while(!(inb(0x1f7)&0x08));

    for(idx=0;idx<256*count;idx++)
    {
        tmpword=(buf[idx*2+1]<<8)|buf[idx*2];
        outw(0x1f0,tmpword);
    }

    return count;
}

int strlen(const char* msg)
{
    int count;
    while(*msg!=0)
    {
        count++;
    }
    count++;
    return count;
}
void outl(u16 port, u32 data)
{
    __asm__ __volatile__("outl %%eax, %%dx"
                        ::"a"(data),"d"(port));
}

u32 inl(u16 port)
{
    u32 value;

    __asm__ __volatile__("inl %%dx, %%eax"
                        :"=a"(value):"d"(port));
                        return value;
}
#define PCI_CONFIG_ADDR 0xcf8
#define PCI_CONFIG_DATA 0xcfc
typedef struct dev
{
    u16 bus;
    u16 dev;
    u16 function;
    u16 vendor_id;
    u16 device_id;
    u16 interrupt;
    u16  int_pin;
    u8 class_code;
    u8 subclass;
}__attribute__((packed)) device;

u32 read(u16 bus, u16 dev, u16 funct, u8 offset)
{
    u32 value=
    1<<31 | bus<<16 | dev<<11 | funct<<8 | offset;
    outl(PCI_CONFIG_ADDR,value);
    u32 result=inl(PCI_CONFIG_DATA);
    return result>>(8*offset%4);

}

device get_device(u16 bus, u16 dev, u16 funct)
{
    device dev_t;
    dev_t.bus=bus;
    dev_t.dev=dev;
    dev_t.function=funct;

    dev_t.vendor_id=read(bus,dev,funct,0x00);
    dev_t.device_id=read(bus,dev,funct,0x02);
    dev_t.interrupt=read(bus,dev,funct,0x3c);
    dev_t.int_pin=read(bus,dev,funct,0x3d);
    dev_t.class_code=read(bus,dev,funct,0x0b);
    dev_t.subclass=read(bus,dev,funct,0x0a);
    return dev_t;
}

void lspci()
{
    for(u16 bus=0;bus<8;bus++)
    {
        for(u16 dev=0;dev<32;dev++)
        {
            for(u16 funct=0;funct<8;funct++)
            {
                device dev_t=get_device(bus,dev,funct);
                if(dev_t.vendor_id==0x0000 || dev_t.vendor_id==0xffff)
                break;
                sys_printf("Bus:");print_hex(dev_t.bus);
                sys_printf(" dev:");print_hex(dev_t.dev);
                sys_printf(" funct:");print_hex(dev_t.function);
                sys_printf(" dev_id:");print_hex(dev_t.device_id);
                sys_printf(" vendor_id:");print_hex(dev_t.vendor_id);
                sys_printf(" interrupt:");print_hex(dev_t.interrupt);
                sys_printf(" int pin:");print_hex(dev_t.int_pin);
                sys_printf(" class:"),print_hex(dev_t.class_code);
                sys_printf(" sub_class:");print_hex(dev_t.subclass);
                sys_printf("\n\n");
            }
        }
    }
}
 void main()
 {
cli();
 char *msg="EOS version 1.0.0 x86_32@mono_core hack_system.\n";
 char color=0x02;
//write_string(msg,0x6|0b00001000);
char string[512];


struct idtr IDTR;
IDTR.limit= IDTSIZE*8-1;
IDTR.base=(u32)&idt;
init_icw();
init_idt();
__asm__ __volatile("lidt %0"::"m"(IDTR));
//memcpy((char*)IDTBASE,(char*)&idt,IDTSIZE);
__asm__ __volatile__("int $0x30"::"b"(msg));
sti();
init_page();
init_dir();

__asm__ __volatile("movl %0, %%cr3;"::"r"(page_directory));

__asm__ __volatile__("movl %%cr0 , %%eax \n"
                    "orl %0, %%eax \n"
                    "movl %%eax, %%cr0"::"i"(0x80000000));
                    write_string("Paging enabled \n",0x02);

__asm__ __volatile__("movl %%cr0 , %%eax \n"
                    "andl %0, %%eax \n"
                    "movl %%eax, %%cr0"::"i"(~0x80000000));
                    write_string("Paging desable \n",0x02);

                    write_string(">E:$ ",0x04);
                  // __asm__("int $0x30"::"b"(msg));
lspci();
                //   switch_to(0x08,(u32)(task1));
                    while(1);

 }
