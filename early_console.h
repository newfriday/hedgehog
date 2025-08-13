#ifndef __ESG_VM_CONSOLE_H__
#define __ESG_VM_CONSOLE_H__
#include "type.h"
#include "string.h"

struct spinlock {
	unsigned int v;
};

static inline void spin_lock(struct spinlock *lock)
{
	while (__sync_lock_test_and_set(&lock->v, 1));
}

static inline void spin_unlock(struct spinlock *lock)
{
	__sync_lock_release(&lock->v);
}

extern void puts(const char *s);
extern uint8_t serial_read(void);

#endif
