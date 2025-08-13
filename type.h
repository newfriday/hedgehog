/*
 * Copyright wgchnln. All rights reserved.
 * function:Basic data type definition(MACRO),header file
 * log:8.1.2019 first create this file
*/

#ifndef __TYPE_H__
#define __TYPE_H__

#define U8      unsigned char
#define S8      char

#define U16     unsigned short
#define I16     short

#define U32     unsigned int
#define I32     int

#define U64     unsigned long long
#define I64     long long

typedef unsigned char uint8_t;                                                                                  
typedef unsigned short uint16_t;                                                                                 
typedef signed int int32_t;
typedef unsigned int uint32_t;
typedef unsigned long uint64_t;
typedef unsigned int size_t;

typedef _Bool bool;

#ifndef NULL
#define         NULL                                ((void *) 0)
#endif

#ifndef true
#define true		((_Bool) 1)
#define false		((_Bool) 0)
#endif

#define OK              0
#define ERROR           (-1)

#define IN
#define OUT
#define INOUT

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

#ifndef UINT64_MAX
#define UINT64_MAX	(0xffffffffffffffffUL)
#endif

#define va_start        __builtin_va_start
#define va_end          __builtin_va_end
typedef __builtin_va_list va_list;

#endif
