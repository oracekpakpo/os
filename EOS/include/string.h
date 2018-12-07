#ifndef _STRING_H__
#define _STRING_H__

#include <types.h>

static inline uchar* strncpy(uchar* dest, const uchar* src, u32 size)
{
    u32 i=0;

    for(i=0; i<size && src[i]; i++)
    {
        dest[i]=src[i];
    }

    for(;i<size;i++)
    {
        dest[i]='\0';
    }

    dest[size]='\0';

    return dest;
}

static inline u32 strlen(const uchar* str)
{
    u32 len=0;

    for(len=0; str[len]; len++);

    return len;
}

static inline int strncmp(const uchar* str1, const uchar* str2, u32 size)
{
    u32 i=0;

    for(i=0; i<size && str1[i]==str2[i]; i++);

    return (i==size) ? 0: ((str1[i]<str2[i]) ? -1 : 1);
}

static inline uchar* strchr(const uchar* str, uchar c)
{
    u32 i=0;

    for(i=0; str[i]; i++)
    {
        if(str[i]==c)
        {
            return (uchar*)(str+i);
        }
    }

    return (uchar*)0;
}

static inline uchar* strrchr(const uchar* str, uchar c)
{
    u32 i=0;

    for(i=strlen(str)-1; i>=0; i--)
    {
        if(str[i]==c)
        {
            return (uchar*)(str+i);
        }
    }

    return (uchar*)NULL;
}

static inline int strcmp(const uchar* str1, const uchar* str2)
{
    return strncmp(str1,str2,strlen(str2));
}

static inline uchar* strcpy(uchar* dest, uchar* src)
{
    return strncpy(dest,src,strlen(src));
}

static inline uchar* strncat(uchar* dest, const uchar* src, u32 size)
{
    u32 dest_size=strlen(dest)-1;

    strncpy(dest+dest_size,src,size);

    return dest;

}

static inline uchar* strcat(uchar* dest, const uchar* src)
{
    u32 src_size=strlen(src)-1;

    return strncat(dest,src,src_size);
}

#endif
