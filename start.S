/*
 * Copyright wgchnln. All rights reserved.
 * function:hypervisor boot
 * log:6.16.2019 first create this file
 *
 * MISRA C requires that all unsigned constants should have the suffix 'U'
 * (e.g. 0xffU), but the assembler may not accept such C-style constants. For
 * example, binutils 2.26 fails to compile assembly in that case. To work this
 * around, all unsigned constants must be explicitly spells out in assembly
 * with a comment tracking the original expression from which the magic
 * number is calculated. As an example:
 *
 *    /* 0x00000668 =
 *     *    (CR4_DE | CR4_PAE | CR4_MCE | CR4_OSFXSR | CR4_OSXMMEXCPT) *\/
 *    movl    $0x00000668, %eax
 *
 * Make sure that these numbers are updated accordingly if the definition of
 * the macros involved are changed.
 */

#if 0
#define START_DBG
#endif

/* MULTIBOOT HEADER */
#define MULTIBOOT_HEADER_MAGIC 0x1badb002
#define MULTIBOOT_HEADER_FLAGS 0x00000002 /*flags bit 1 : enable mem_*, mmap_**/

    .section    multiboot_header, "a"

    .align     4

    /* header magic */
    .long   MULTIBOOT_HEADER_MAGIC
    /* header flags - flags bit 6 : enable mmap_* */
    .long   MULTIBOOT_HEADER_FLAGS
    /* header checksum = -(magic + flags) */
    .long   -(MULTIBOOT_HEADER_MAGIC + MULTIBOOT_HEADER_FLAGS)

    .section    entry, "ax"

    .align      8
    .code32

    .global     cpu_primary_start_32
cpu_primary_start_32:
#ifdef START_DBG
	mov $0x3f8,%edx
	mov $0x31, %eax
	out %al,  (%dx)
	mov $0x3fd,%edx
__debug1__:
	in  (%dx),%al
	test   $0x20,%al
	je __debug1__
#endif

    /* save the MULTBOOT magic number & MBI */
    movl    %eax, (boot_regs)
    movl    %ebx, (boot_regs+4)

    /* Disable interrupts */
    cli
    /* Clear direction flag */
    cld

    /* detect whether it is in long mode
     *
     *     0xc0000080 = MSR_IA32_EFER
     */
    movl    $0xc0000080, %ecx
    rdmsr
    /* 0x400 = MSR_IA32_EFER_LMA_BIT */
    test     $0x400, %eax
    /* jump to 64bit entry if it is already in long mode */
    jne      cpu_primary_start_64

    /* Disable paging */
    mov     %cr0, %ebx
    /* 0x7fffffff = ~CR0_PG */
    andl    $0x7fffffff, %ebx
    mov     %ebx, %cr0

    /* Set DE, PAE, MCE and OS support bits in CR4
     * 0x00000668 =
     *    (CR4_DE | CR4_PAE | CR4_MCE | CR4_OSFXSR | CR4_OSXMMEXCPT) */
    movl    $0x00000668, %eax
    mov     %eax, %cr4

    /* Set CR3 to PML4 table address */
    movl    $cpu_boot32_page_tables_start, %edi
    mov     %edi, %cr3

    /* Set LME bit in EFER */
    /* 0xc0000080 = MSR_IA32_EFER */
    movl    $0xc0000080, %ecx
    rdmsr
    /* 0x00000100 = MSR_IA32_EFER_LME_BIT */
    orl     $0x00000100, %eax
    wrmsr

    /* Enable paging, protection, numeric error and co-processor
       monitoring in CR0 to enter long mode */
    mov     %cr0, %ebx
    /* 0x80000023 = (CR0_PG | CR0_PE | CR0_MP | CR0_NE) */
    orl     $0x80000023, %ebx
    mov     %ebx, %cr0

    /* Load temportary GDT pointer value */
    mov     $gdt64_desc, %ebx
    lgdt    (%ebx)

    /* 0x10 = HOST_GDT_RING0_DATA_SEL*/
    movl    $0x10,%eax
    mov     %eax,%ss  /* Was 32bit POC Stack*/
    mov     %eax,%ds  /* Was 32bit POC Data*/
    mov     %eax,%es  /* Was 32bit POC Data*/
    mov     %eax,%fs  /* Was 32bit POC Data*/
    mov     %eax,%gs  /* Was 32bit POC CLS*/

    /* Perform a long jump based to start executing in 64-bit mode */
    /* 0x0008 = HOST_GDT_RING0_CODE_SEL */
    ljmp    $0x0008, $primary_start_long_mode

    .code64
    .org 0x200
    .global     cpu_primary_start_64
cpu_primary_start_64:
#ifdef START_DBG
	mov $0x3f8,%edx
	mov $0x32, %eax
	out %al,  (%dx)
	mov $0x3fd,%edx
__debug2__:
	in  (%dx),%al
	test   $0x20,%al
	je __debug2__
#endif

    /* save the MULTBOOT magic number & MBI */
    lea     boot_regs(%rip), %rax
    movl    %edi, (%rax)
    movl    %esi, 4(%rax)

primary_start_long_mode:

    /* Initialize temporary stack pointer */
    lea     ld_bss_end(%rip), %rsp
    /*0x1000 = PAGE_SIZE*/
    add     $0x1000,%rsp
    /* 16 = CPU_STACK_ALIGN */
    and     $(~(16 - 1)),%rsp

    /* detect whether it is in long mode
     *
     *     0xc0000080 = MSR_IA32_EFER
     */
    movl    $0xc0000080, %ecx
    rdmsr
    /* 0x400 = MSR_IA32_EFER_LMA_BIT */
    test     $0x400, %eax
    /* jump to 64bit entry if it is already in long mode */
    jne      is_long_mode
#ifdef START_DBG
	mov $0x3f8,%edx
	mov $0x33, %eax
	out %al,  (%dx)
	mov $0x3fd,%edx
__debug3__:
	in  (%dx),%al
	test   $0x20,%al
	je __debug3__
#endif

loop:
    jmp loop

is_long_mode:
    call main
    jmp loop

.data
    .align  4
    .global boot_regs
boot_regs:
    .long   0x00000000
    .long   0x00000000

/* PML4, PDPT, and PD tables initialized to map first 4 GBytes of memory */
    /*0x1000 = PAGE_SIZE*/
    .align  0x1000
    .global cpu_boot32_page_tables_start
cpu_boot32_page_tables_start:
    /* 0x3 = (PAGE_PRESENT | PAGE_RW) */
    .quad   cpu_primary32_pdpt_addr + 0x3
    /*0x1000 = PAGE_SIZE*/
    .align  0x1000
cpu_primary32_pdpt_addr:
    address = 0
    .rept   4
    /* 0x3 = (PAGE_PRESENT | PAGE_RW) */
    .quad   cpu_primary32_pdt_addr + address + 0x3
    /*0x1000 = PAGE_SIZE*/
    address = address + 0x1000
    .endr
    /*0x1000 = PAGE_SIZE*/
    .align  0x1000
cpu_primary32_pdt_addr:
    address = 0
    .rept  2048
    /* 0x83 = (PAGE_PSE | PAGE_PRESENT | PAGE_RW) */
    .quad  address + 0x83
    address = address + 0x200000
    .endr

.align 4096

gdt64_desc:
	.word gdt64_end - gdt64 - 1
	.quad gdt64

gdt64:
	.quad 0
	.quad 0x00af9b000000ffff // 64-bit code segment
	.quad 0x00cf93000000ffff // 32/64-bit data segment
	.quad 0x00af1b000000ffff // 64-bit code segment, not present
	.quad 0x00cf9b000000ffff // 32-bit code segment
	.quad 0x008f9b000000FFFF // 16-bit code segment
	.quad 0x008f93000000FFFF // 16-bit data segment
	.quad 0x00cffb000000ffff // 32-bit code segment (user)
	.quad 0x00cff3000000ffff // 32/64-bit data segment (user)
	.quad 0x00affb000000ffff // 64-bit code segment (user)

	.quad 0			 // 6 spare selectors
	.quad 0
	.quad 0
	.quad 0
	.quad 0
	.quad 0
gdt64_end:

