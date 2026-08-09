ENTRY(Reset_Handler)

MEMORY{
    FLASH(rx) : ORIGIN = 0x08000000, LENGTH = 512k
    RAM(rwx) : ORIGIN = 0x20000000, LENGTH = 96k
}

SECTIONS{
    .isr_vector : 
    {
        KEEP(*(.isr_vector))
    } > FLASH

    .text :
    {
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
    } > FLASH

    .data :
  {
    _sdata = .;
    *(.data)
    *(.data*)
    _edata = .;
  } > RAM AT> FLASH

  _sidata = LOADADDR(.data);

  .bss :
  {
    _sbss = .;
    *(.bss)
    *(.bss*)
    *(COMMON)
    _ebss = .;
  } > RAM

  ._user_heap_stack :
  {
    . = . + 0x200;
    . = . + 0x400;
  } > RAM

    _estack = ORIGIN(RAM)+LENGTH(RAM)
}