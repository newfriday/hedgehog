#include "early_print.h"
#include "early_console.h"
#include "string.h"
#include "type.h"

#define BUFSZ 2000

#define MIN(a, b)		(((a) < (b)) ? (a) : (b))
#define MAX(a, b)		(((a) > (b)) ? (a) : (b))
#define BINSTR_SZ (sizeof(unsigned long) * 8 + sizeof(unsigned long) * 2)

#define va_arg(v, l)	__builtin_va_arg(v, l)

typedef struct pstream {
	char *buffer;
	int remain;
	int added;
} pstream_t;

typedef struct strprops {
	char pad;
	int npad;
	bool alternate;
} strprops_t;

static long atol(const char *ptr)
{
	long acc = 0;
	const char *s = ptr;
	int neg, c;

	while ((*s == ' ') || (*s == '\t')) {
		s++;
	}
	if (*s == '-') {
		neg = 1;
		s++;
	} else {
		neg = 0;
		if (*s == '+') {
			s++;
		}
	}

	while (*s) {
		if ((*s < '0') || (*s > '9')) {
			break;
		}
		c = *s - '0';
		acc = acc * 10 + c;
		s++;
	}

	if (neg) {
		acc = -acc;
	}

	return acc;
}

static void addchar(pstream_t *p, char c)
{
	if (p->remain) {
		*p->buffer++ = c;
		--p->remain;
	}
	++p->added;
}

static void print_str(pstream_t *p, const char *s, strprops_t props)
{
	const char *s_orig = s;
	const char *ps = s;
	int npad = props.npad;

	if (npad > 0) {
		npad -= strlen(s_orig);
		while (npad > 0) {
			addchar(p, props.pad);
			--npad;
		}
	}

	while (*ps) {
		addchar(p, *ps++);
	}

	if (npad < 0) {
		props.pad = ' '; /* ignore '0' flag with '-' flag */
		npad += strlen(s_orig);
		while (npad < 0) {
			addchar(p, props.pad);
			++npad;
		}
	}
}

static char digits[16] = "0123456789abcdef";

static void print_int(pstream_t *ps, long long n, int base, strprops_t props)
{
	char buf[sizeof(long) * 3 + 2], *p = buf;
	long long nm = n;
	int s = 0, i;

	if (nm < 0) {
		nm = -nm;
		s = 1;
	}

	while (nm) {
		*p++ = digits[nm % base];
		nm /= base;
	}

	if (s) {
		*p++ = '-';
	}

	if (p == buf) {
		*p++ = '0';
	}

	for (i = 0; i < ((p - buf) / 2); ++i) {
		char tmp;

		tmp = buf[i];
		buf[i] = p[-1-i];
		p[-1-i] = tmp;
	}

	*p = 0;

	print_str(ps, buf, props);
}

static void print_unsigned(pstream_t *ps, unsigned long long n, int base,
	strprops_t props)
{

	char buf[sizeof(long) * 3 + 3], *p = buf;
	strprops_t *p_props = (strprops_t *)&props;
	unsigned long long nm = n;
	int i;

	while (nm) {
		*p++ = digits[nm % base];
		nm /= base;
	}

	if (p == buf) {
		*p++ = '0';
	} else if (p_props->alternate && (base == 16)) {
		if (p_props->pad == '0') {
			addchar(ps, '0');
			addchar(ps, 'x');

			if (p_props->npad > 0) {
				p_props->npad = MAX((p_props->npad - 2), 0);
			}
		} else {
			*p++ = 'x';
			*p++ = '0';
		}
	}

	for (i = 0; i < ((p - buf) / 2); ++i) {
		char tmp;

		tmp = buf[i];
		buf[i] = p[-1-i];
		p[-1-i] = tmp;
	}

	*p = 0;

	print_str(ps, buf, props);
}

static int fmtnum(const char **fmt)
{
	const char *f = *fmt;
	int len = 0, num;

	if (*f == '-') {
		++f, ++len;
	}

	while ((*f >= '0') && (*f <= '9')) {
		++f, ++len;
	}

	num = atol(*fmt);
	*fmt += len;
	return num;
}

int evsnprintf(char *buf, int size, const char *fmt, va_list va)
{
	const char *pfmt = fmt;
	pstream_t s;

	s.buffer = buf;
	s.remain = size - 1;
	s.added = 0;
	while (*pfmt) {
		char f = *pfmt++;
		int nlong = 0;
		strprops_t props;
		memset(&props, 0, sizeof(props));
		props.pad = ' ';

		if (f != '%') {
			addchar(&s, f);
			continue;
		}
morefmt:
		f = *pfmt++;
		switch (f) {
		case '%':
			addchar(&s, '%');
			break;
		case 'c':
			addchar(&s, va_arg(va, int));
			break;
		case '\0':
			--pfmt;
			break;
		case '#':
			props.alternate = true;
			goto morefmt;
		case '0':
			props.pad = '0';
			++pfmt;
		/* fall through */
		case '1'...'9':
		case '-':
			--pfmt;
			props.npad = fmtnum(&pfmt);
			goto morefmt;
		case 'l':
			++nlong;
			goto morefmt;
		case 't':
		case 'z':
			/* Here we only care that sizeof(size_t) == sizeof(long).
			 * On a 32-bit platform it doesn't matter that size_t is
			 * typedef'ed to int or long; va_arg will work either way.
			 * Same for ptrdiff_t (%td).
			 */
			nlong = 1;
			goto morefmt;
		case 'd':
			switch (nlong) {
			case 0:
				print_int(&s, va_arg(va, int), 10, props);
				break;
			case 1:
				print_int(&s, va_arg(va, long), 10, props);
				break;
			default:
				print_int(&s, va_arg(va, long long), 10, props);
				break;
			}
			break;
		case 'u':
			switch (nlong) {
			case 0:
				print_unsigned(&s, va_arg(va, unsigned), 10, props);
				break;
			case 1:
				print_unsigned(&s, va_arg(va, unsigned long), 10, props);
				break;
			default:
				print_unsigned(&s, va_arg(va, unsigned long long), 10, props);
				break;
			}
			break;
		case 'x':
			switch (nlong) {
			case 0:
				print_unsigned(&s, va_arg(va, unsigned), 16, props);
				break;
			case 1:
				print_unsigned(&s, va_arg(va, unsigned long), 16, props);
				break;
			default:
				print_unsigned(&s, va_arg(va, unsigned long long), 16, props);
				break;
			}
			break;
		case 'p':
			props.alternate = true;
			print_unsigned(&s, (unsigned long)va_arg(va, void *), 16, props);
			break;
		case 's':
			print_str(&s, va_arg(va, const char *), props);
			break;
		default:
			addchar(&s, f);
			break;
		}
	}
	*s.buffer = 0;
	return s.added;
}


int esnprintf(char *buf, int size, const char *fmt, ...)
{
	va_list va;
	int r;

	va_start(va, fmt);
	r = evsnprintf(buf, size, fmt, va);
	va_end(va);
	return r;
}

int evprintf(const char *fmt, va_list va)
{
	char buf[BUFSZ];
	int r;

	r = evsnprintf(buf, sizeof(buf), fmt, va);
	puts(buf);
	return r;
}

int printf(const char *fmt, ...)
{
	va_list va;
	char buf[BUFSZ];
	int r;

	va_start(va, fmt);
	r = evsnprintf(buf, sizeof buf, fmt, va);
	va_end(va);
	puts(buf);
	return r;
}

void binstr(unsigned long x, char out[BINSTR_SZ])
{
	int i;
	char *c;
	int n;

	n = sizeof(unsigned long) * 8;
	i = 0;
	c = &out[0];
	for (;;) {
		*c++ = (x & (1ul << (n - i - 1))) ? '1' : '0';
		i++;

		if (i == n) {
			*c = '\0';
			break;
		}
		if ((i % 4) == 0) {
			*c++ = '\'';
		}
	}
}

void print_binstr(unsigned long x)
{
	char out[BINSTR_SZ];
	binstr(x, out);
	printf("%s", out);
}

