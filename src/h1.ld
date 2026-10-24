ENTRY(h1_bda_main)
SECTIONS {
 . = 0x83c00020;
 .entry : { KEEP(*(.text.h1_bda_entry)) }
 .text : { *(.text*) }
 .rodata : { *(.rodata*) }
 .data : { *(.data*) *(.sdata*) }
 . = ALIGN(16);
 .bss (NOLOAD) : {
  __bss_start = .;
  *(.bss*) *(.sbss*) *(COMMON)
  . = ALIGN(16);
  . += 0x10000;
  __app_stack_top = .;
  __bss_end = .;
 }
 ASSERT(__bss_end <= 0x83ff0000, "H1 application window exceeded")
 ASSERT(h1_bda_main == 0x83c00020, "H1 BDA entry alignment moved")
 /DISCARD/ : { *(.MIPS.abiflags) *(.reginfo) *(.eh_frame*) *(.comment) *(.note*) *(.pdr) }
}
