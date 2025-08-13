ENTRY(cpu_primary_start_32)
MEMORY
{
 lowram : ORIGIN = 0, LENGTH = 0x00010000
 ram : ORIGIN = 0x100000, LENGTH = 0x2000000
}
SECTIONS
{
 .boot :
 {
  KEEP(*(multiboot_header)) ;
 } > ram
 .entry :
 {
  KEEP(*(entry)) ;
 } > ram
 .text :
 {
  *(.text .text*) ;
  *(.gnu.linkonce.t*)
  *(.note.gnu.build-id)
  *(.retpoline_thunk)
 } > ram
 . = ALIGN(0x200000);
 .rodata :
 {
  *(.rodata*) ;
 } > ram
 .rela :
 {
  *(.rela*)
  *(.dyn*)
 } > ram
 . = ALIGN(4) ;
 .data :
 {
  *(.data) ;
  *(.data*) ;
  *(.sdata)
  *(.gnu.linkonce.d*)
 } > ram
 .bss_noinit (NOLOAD):
 {
  . = ALIGN(4) ;
  *(.bss_noinit) ;
  *(.bss_noinit*) ;
  . = ALIGN(4) ;
 } > ram
 .bss (NOLOAD):
 {
  . = ALIGN(4) ;
  ld_bss_start = . ;
  *(.bss) ;
   *(.bss*) ;
  *(COMMON) ;
  . = ALIGN(4) ;
  ld_bss_end = . ;
 } > ram
}
