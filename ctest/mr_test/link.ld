OUTPUT_ARCH( "riscv" )
ENTRY(_start)

MEMORY
{
    RAM (rwx)  : ORIGIN = 0x00001000, LENGTH = 0x040000000
}

_stack_len = 0x010000;
_min_heap_size = 0x200000;

SECTIONS
{
  . = 0x00001000;
  .text.init : {
	*(.text.init)
    _init_end = .;
  } > RAM 
  .text : {
    _text_start = .;
    *(.text)
    _text_end = .;
  } > RAM 
  .rodata : {
    _rodata_start = .;
    *(.rodata)
    *(.rodata.*)
    _rodata_end = .;
  } > RAM
  .srodata : {
    _srodata_start = .;
    *(.srodata.*)
    *(.srodata)
    _srodata_end = .;
  } > RAM
  .data : {
    _data_start = .;
    __global_pointer$ = . + 0x800;
    *( .data)
    _data_end = .;
  } > RAM 
  
  .sbss : {
    . = ALIGN(16);
    *(.sbss .sbss.*)
  }
  .bss :{
    . = ALIGN(16);
    _bss_start = .;
    *(.bss)
    _bss_end = .;
  } > RAM 

  .heap :{
    PROVIDE(_heap_start = .);
	. = . + _min_heap_size;
    PROVIDE(_heap_end = .);
  } > RAM 

  .stack :{
    PROVIDE(_stack_start = .);
    . = . + _stack_len;
    PROVIDE(_stack_end = .);
    stack = . ;
    _stack = . ;
  } > RAM 

  _end = .;
}

