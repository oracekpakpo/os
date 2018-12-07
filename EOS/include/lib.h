#ifndef __LIB_H__
#define __LIB_H__

#include <types.h>
#include <string.h>


void* memcpy(void* dest, void* src, u32 size)
{

    u32 i=0;

    for(i=0; i<size; i++)
    {
        ((uchar*)dest)[i]=((uchar*)src)[i];
    }

    return dest;
}


void* memset(void* dest, uchar data, u32 size)
{
    u32 i=0;

    for(i=0;i<size;i++)
    {
       ((uchar*)dest)[i]=data;
    }

    return dest;
}

u32 memcmp(const void* str1, const void* str2, u32 size)
{
    u32 i=0;

    for(i=0; i<size && ((uchar*)str1)[i]==((uchar*)str2)[i]; i++);

    return (i==size) ? 0: ((((uchar*)str1)[i] < ((uchar*)str2)[i]) ? -1 : 1);
}

#endif
