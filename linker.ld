ENTRY(start)
SECTIONS
{
  . = 0x00100000;

  _kernel_start = .;

  .multiboot_header :
  {
    *(.multiboot_header)
  }

  .text :
  {
    *(.text)
  }

  .data :
  {
    *(.data)
  }

  .bss :
  {
    *(.bss)
    *(COMMON)
  }

  _kernel_end = .;
}
