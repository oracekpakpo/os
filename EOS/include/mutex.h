#ifndef __MUTEX_H__
#define __MUTEX_H__
#include <x86/idt.h>

#define INIT_MUTEX(mutex) \
(mutex)->mutex_lock = mutex_lock; \
(mutex)->mutex_unlock = mutex_unlock;

struct mutex
{
	void (*mutex_lock)(void);
	void (*mutex_unlock)(void);
};

static inline void mutex_lock(void)
{
	mask_irq(IRQ_0);
	mask_irq(IRQ_1);
}

static inline void mutex_unlock(void)
{
	enable_irq();
}

#endif //__MUTEX_H__