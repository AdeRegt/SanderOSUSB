ENTRY(_start)
 
 SECTIONS
{
  . = 0x5000;
  .text : { *(.text) }
  .data : { *(.data) }
  .bss : { *(.bss) }
}
