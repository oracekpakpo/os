#ifndef __IO__H__
#define __IO__H__

#include <types.h>
#include <asm_io.h>
#include <color.h>
#include <string.h>

void write_char(uchar, u32);
void write_string(const uchar* ,u32);
void scrollup(u32);
void perror(const uchar*);
void change_cursor(u32, u32);
void move_cursor(u8,u8);
void show_cursor();
void print_hex(u32 n);
void kprintf(const uchar* str, ...);

#endif

