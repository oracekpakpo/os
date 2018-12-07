#include <io.h>
#include <color.h>

int cx = 0;
int cy = 1;

void change_cursor(u32 x, u32 y)
{
    cx=x; cy=y;
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
    move_cursor(cx,cy);
}

void write_string(const uchar* string, u32 color)
{
    while(*string !=0)
    {
       write_char(*string,color);
      string++;
    }
}

void scrollup(u32 n)
{

    volatile memory video;
    volatile memory tmp;

    for(memory i = (memory)0xB8000; i < (memory)0xb8fa0; i+=2)
    {
        tmp = (volatile memory)(i + n*160);
        video = (volatile memory)i;

        if(tmp <(memory)0xb8fa0)
        {
            *video = *tmp;
            *(video+1) = *(tmp+1);
        }
        else
        {
            *video = 0;
            *(video+1) = 0x07;
        }

    }

    cy -= n;
    if(cy < 0) 
        cy=0;
}


void write_char(uchar c, u32 color)
{
    if(c == 10)
    {
        cx = 0;
        cy++;
    }
    else if(c == 9)
    {
        cx = cx + 4;
    }
    else if(c == 13)
    {
        cx = 0;
    }
    else
    {
        volatile memory video = (volatile memory)(0xb8000 + 2*cx + 160*cy);
        *video = c;
        *(video+1) = color;
        cx++;

    if(cx > 79)
    {
        cx = 0;
        cy++;
    }
    if(cy > 24)
    {
        scrollup(cy-24);
    }
        show_cursor();
    }
}
/* Background color black foreground green*/

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
    write_string(format,black<<4|white);
}

void kprintf(const uchar* str,...)
{
    va_list list;
    va_start(list,str);

    for(int i=0; i<strlen(str); i++)
    {
        if(str[i]=='%')
        {
            if(str[i+1]=='x')
            {
                u32 arg=va_arg(list,u32);
                print_hex(arg);
                i+=1;
            }else if(str[i+1]=='s')
            {
                uchar* msg=va_arg(list,uchar*);
                write_string(msg,black<<4|white);
                i+=1;
            }else if(str[i+1]=='c')
            {
                char c=va_arg(list,char);
                write_char(c,black<<4|white);
                i+=1;
            }else
            {
                write_char(str[i],black<<4|white);
            }
        }
        else
        {
            write_char(str[i],black<<4|white);
        }

    }
    va_end(list);
}

void perror(const uchar* str)
{
    write_string(str,black << 4 | red);
}
