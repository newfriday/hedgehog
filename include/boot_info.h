#ifndef __BOOT_INFO_H__
#define __BOOT_INFO_H__

#include "type.h"

void start_kernel(void);

void kernel_entry(uint64_t kernel_entry_addr);

void dump_mem(uint32_t start, uint32_t len);


#endif
