/*
 * Copyright wgchnln. All rights reserved.
 * function:String processing function ,header file
 * log:8.1.2019 first create this file
*/

#ifndef __STRING_H__
#define __STRING_H__

#include "./type.h"

extern int strlen(const char *str);
extern void memset(void *dstv, char ch, U32 size);
extern void memcpy(char *dst, const char *src, U32 size);
extern I32 strncmp(const char *s1_arg, const char *s2_arg, size_t n_arg);
extern I32 strcmp(const char *s1_arg, const char *s2_arg);
extern size_t strnlen_s(const char *str_arg, size_t maxlen_arg);


#endif
