#include "boot_info.h"
#include "early_print.h"
#include "multi_boot.h"
#include "string.h"
#include "type.h"
#include "i8259.h"

void start_kernel(void)
{
    printf("\nstart kernel ...\n");

    init_8259A();

    printf("init 8259A PIC...\n");

    while(1){}
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

