/*
 * Copyright wgchnln. All rights reserved.
 * function:String processing function,C file
 * log:8.1.2019 first create this file
*/

#include "./string.h"

int strlen(const char *str)
{
	U32 n;

	for (n = 0; *str; ++str){
		++n;
    }
	return n;
}

void memset(void *dstv, char ch, U32 size)
{
	char *dst = dstv;
	U32 i;

	for (i = 0; i < size; i++)
		dst[i] = ch;
}

void memcpy(char *dst, const char *src, U32 size)
{
	U32 i;

	for (i = 0; i < size; i++)
		*dst++ = *src++;
}

I32 strncmp(const char *s1_arg, const char *s2_arg, size_t n_arg)
{
	const char *str1 = s1_arg;
	const char *str2 = s2_arg;
	size_t n = n_arg;

	while (((n - 1) != 0U) && ((*str1) != '\0') && ((*str2) != '\0') && ((*str1) == (*str2))) {
		str1++;
		str2++;
		n--;
	}

	return *str1 - *str2;
}

I32 strcmp(const char *s1_arg, const char *s2_arg)
{
	const char *str1 = s1_arg;
	const char *str2 = s2_arg;

	while (((*str1) != '\0') && ((*str2) != '\0') && ((*str1) == (*str2))) {
		str1++;
		str2++;
	}

	return *str1 - *str2;
}

size_t strnlen_s(const char *str_arg, size_t maxlen_arg)
{
	const char *str = str_arg;
	size_t count = 0U;

	if (str != NULL) {
		size_t maxlen = maxlen_arg;
		while ((*str) != '\0') {
			if (maxlen == 0U) {
				break;
			}

			count++;
			maxlen--;
			str++;
		}
	}

	return count;
}