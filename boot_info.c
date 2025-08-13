#include "boot_info.h"
#include "early_print.h"
#include "multi_boot.h"
#include "string.h"
#include "type.h"


/*
 * +-----------------+
 * | / / / / / / / / |
 * | / / / / / / / / |
 * | / / / / / / / / |
 * | / / / / / / / / |
 * |  / yuan kernel /|
 * +-/-/-image-/-/-/-+ 0x0020_b000 multiboot module load address(kernel image)---+
 * | / / / / / / / / |                                                           |
 * +-----------------+ 0x0020_0000 kernel relocation load address <--------------+
 * | \ \ \ \ \ \ \ \ |
 * |  \ \ \ \ \ \ \  |
 * | yuan bootloader |
 * | \ \  image \ \  |
 * +-----------------+ 0x0010_0000 yuan bootloader load address
 * |                 |
 * |                 |
 * |                 |
 * +-----------------+ 0x0001_0000 multiboot info
 * |                 |
 * +-----------------+ 0x0000_0000
 *
 */

#define MAX_MOD_TAG_LEN      32U
#define INVALID_MOD_IDX 0xFFFFU


#define DEBUG_KERNEL_ADDR		0


#define KERNEL_START			0x200000
#define KERNEL_ENTRY_OFFSET		0x10

#define KERNEL_X86_64			1

typedef void  (*__entry)(void);


static uint32_t get_mod_idx_by_tag(const struct multiboot_module *mods, uint32_t mods_count, const char *tag)
{
	uint32_t i, ret = INVALID_MOD_IDX;
	uint32_t tag_len = strnlen_s(tag, MAX_MOD_TAG_LEN);

	/** For each i ranging from 0 to mods_count [with a step of 1] */
	for (i = 0U; i < mods_count; i++) {
		const char *mm_string = (char *)(uint64_t)mods[i].mm_string;
		uint32_t mm_str_len = strnlen_s(mm_string, MAX_MOD_TAG_LEN);

		if ((mm_str_len >= tag_len) && (strncmp(mm_string, tag, tag_len) == 0) &&
			((*(mm_string + tag_len) == 0x0d) || (*(mm_string + tag_len) == 0x0a) ||
				(*(mm_string + tag_len) == 0))) {
			ret = i;
			break;
		}
	}
	return ret;
}


static uint64_t get_kernel_load_addr(void)
{
	return KERNEL_START;
}

static void init_vm_kernel_info(const struct multiboot_module *mod)
{
	uint64_t kernel_load_addr ;
	uint64_t kernel_src_addr;
	uint32_t kernel_size;
	__entry _entry;

	kernel_src_addr = (uint64_t)mod->mm_mod_start;
	kernel_size = mod->mm_mod_end - mod->mm_mod_start;
	kernel_load_addr = get_kernel_load_addr();

	printf("\nmultiboot info:kernel start addr:0x%x, end addr:0x%x, size:%d Bytes", kernel_src_addr, mod->mm_mod_end, kernel_size);
	printf("\nrelocation kernel load addr:0x%x", kernel_load_addr);
#if DEBUG_KERNEL_ADDR
	printf("multiboot info @ [0x%08x,0x%08x]", boot_regs[0], boot_regs[1]);
	dump_mem(kernel_src_addr, 0x100);
#endif
	printf("\nCopy kernel os...");
	memcpy((char *)kernel_load_addr, (char *)kernel_src_addr, kernel_size);
#if DEBUG_KERNEL_ADDR
	dump_mem(kernel_load_addr, 0x100);
#endif

	printf("\nEnter kernel...\n\n\n\n\n\n\n");

#if 0
	_entry = (__entry)(kernel_load_addr+0x10);
	(*_entry)();
#else
	_entry = (__entry)(kernel_load_addr+KERNEL_ENTRY_OFFSET);
	kernel_entry((uint64_t)_entry);
#endif
}

void kernel_entry(uint64_t kernel_entry_addr)
{
#if KERNEL_X86_64
	asm volatile("movl %0, %%edi" : : "c"(boot_regs[0]) : );
	asm volatile("movl %0, %%esi" : : "c"(boot_regs[1]) : );
#else
	asm volatile("movl %0, %%eax" : : "c"(boot_regs[0]) : );
	asm volatile("movl %0, %%ebx" : : "c"(boot_regs[1]) : );
#endif
	asm volatile("call %0" : : "c"(kernel_entry_addr) : );
}



void start_kernel(void)
{
	struct multiboot_info *mbi = (struct multiboot_info *)(uint64_t)boot_regs[1];
	struct multiboot_module *mods;
	uint32_t mod_idx;

	printf("\nMultiboot detected, flag=0x%x", mbi->mi_flags);

	mods = (struct multiboot_module *)(uint64_t)mbi->mi_mods_addr;

	printf("\n module addr @0x%x, module counts=%d\n", mods, mbi->mi_mods_count);

	mod_idx = get_mod_idx_by_tag(mods, mbi->mi_mods_count, "yuan-os");
	
	init_vm_kernel_info(&mods[mod_idx]);
}

void dump_mem(uint32_t start, uint32_t len)
{
	unsigned int i;

	printf("\nkernel data(start:0x%x, len:0x%x):\n", start, len);
	for (i=start ; i<(start+len) ; i+=2) {
		if (!((i-start)%16)) {
			printf("\n");
		}
		printf("%04x ", *((uint16_t *)(uint64_t)i));
	}
	printf("\n");
}

