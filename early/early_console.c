#include "early_console.h"

static struct spinlock lock;
static int serial_iobase = 0x3f8;
static int serial_inited = 0;

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

static void serial_outb(char ch)
{
	uint8_t lsr;

	do {
		lsr = inb(serial_iobase + 0x05);
	} while (!(lsr & 0x20));

	outb(ch, serial_iobase + 0x00);
}

static void serial_put(char ch)
{
	/* Force carriage return to be performed on \n */
	if (ch == '\n') {
		serial_outb('\r');
	}
	serial_outb(ch);
}

static void serial_init(void)
{
	uint8_t lcr;

	/* set DLAB */
	lcr = inb(serial_iobase + 0x03);
	lcr |= 0x80;
	outb(lcr, serial_iobase + 0x03);

	/* set baud rate to 115200 */
	outb(0x01, serial_iobase + 0x00);
	outb(0x00, serial_iobase + 0x01);

	/* clear DLAB */
	lcr = inb(serial_iobase + 0x03);
	lcr &= ~0x80;
	outb(lcr, serial_iobase + 0x03);

	/* IER: disable interrupts */
	outb(0x00, serial_iobase + 0x01);
	/* LCR: 8 bits, no parity, one stop bit */
	outb(0x03, serial_iobase + 0x03);
	/* FCR: disable FIFO queues */
	outb(0x00, serial_iobase + 0x02);
	/* MCR: RTS, DTR on */
	outb(0x03, serial_iobase + 0x04);
}

static void print_serial(const char *buf)
{
	unsigned long len = strnlen_s(buf, 4096);

	unsigned long i;
	if (!serial_inited) {
		serial_init();
		serial_inited = 1;
	}

	for (i = 0; i < len; i++) {
		serial_put(buf[i]);
	}
}

void puts(const char *s)
{
	spin_lock(&lock);
	print_serial(s);
	spin_unlock(&lock);
}

uint8_t serial_read(void)
{
	if (!(inb(serial_iobase + 0x05) & 0x01))
		return 0;

	return inb(serial_iobase + 0x00);
}


