#ifndef __TIME_H__
#define __TIME_H__
#include <types.h>

typedef struct
{
    u32_t year;
    u32_t month;
    u32_t date;
    u32_t hour;
    u32_t minute;
    u32_t second;
}time_t;

static inline void local_time(time_t* __time)
{
	
}

#endif //__TIME_H__