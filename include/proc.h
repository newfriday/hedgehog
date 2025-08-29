

typedef struct s_stackframe {
    U64 gs;     /* \                                    */
    U64 fs;     /* |                                    */
    U64 es;     /* |                                    */
    U64 ds;     /* |                                    */
    U64 edi;        /* |                                    */
    U64 esi;        /* | pushed by save()                   */
    U64 ebp;        /* |                                    */
    U64 kernel_esp; /* <- 'popad' will ignore it            */
    U64 ebx;        /* |                                    */
    U64 edx;        /* |                                    */
    U64 ecx;        /* |                                    */
    U64 eax;        /* /                                    */
    U64 retaddr;    /* return addr for kernel.asm::save()   */
    U64 eip;        /* \                                    */
    U64 cs;     /* |                                    */
    U64 eflags;     /* | pushed by CPU during interrupt     */
    U64 esp;        /* |                                    */
    U64 ss;     /* /                                    */
}STACK_FRAME;
