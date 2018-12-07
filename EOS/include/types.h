#ifndef __TYPES__H
#define __TYPES__H

typedef unsigned char uchar;
typedef unsigned char u8;
typedef unsigned char byte;
typedef unsigned char* memory;
typedef unsigned char bool;
typedef unsigned char u8_t;
typedef signed char s8_t;

typedef unsigned short ushort;
typedef unsigned short u16;
typedef unsigned short word;
typedef unsigned short u16_t;
typedef signed short s16_t;

typedef unsigned long uint;
typedef unsigned long u32;
typedef unsigned long dword;
typedef unsigned long u32_t;
typedef signed long s32_t;


#define ERROR -1
#define OK 1
#define NO 0
#define MAX_LONG 0xffffffff
#define MAX_SHORT 0xffff
#define MAX_CHAR 0xff
#define NULL (void*)0x00
#define PATH_MAX_LEN 512
#define MAX_FILE 30

#define va_start(v, l)	__builtin_va_start(v, l)
#define va_arg(v, l)	__builtin_va_arg(v, l)
#define va_end(v)	__builtin_va_end(v)
#define va_copy(d, s)	__builtin_va_copy(d, s)
typedef __builtin_va_list va_list;


#endif // __TYPES__H

