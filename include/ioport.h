#include "type.h"

static inline uint8_t inb(unsigned long port)
{
    unsigned char value;
    asm volatile("inb %w1, %0" : "=a" (value) : "Nd" ((unsigned short)port));
    return value;
}

static inline void outb(uint8_t value, unsigned long port)
{
    asm volatile("outb %b0, %w1" : : "a"(value), "Nd"((unsigned short)port));
}
