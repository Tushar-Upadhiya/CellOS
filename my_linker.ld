ENTRY(Reset_Handler)

MEMORY{
    FLASH(rx) : ORIGIN = 0x08000000, LENGTH = 512k
    RAM(rwx) : ORIGIN = 0x20000000, LENGTH = 96k
}

_Heap_Size      = 0x4000;   /* 16 KB for the allocator, adjust later */
_Min_Stack_Size = 0x400;    /* minimum left for the main (MSP) stack */

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
        . = ALIGN(4);
    } > FLASH

    .data :
    {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > RAM AT> FLASH

    _sidata = LOADADDR(.data);

    .bss :
    {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > RAM

    .heap (NOLOAD) :
    {
        . = ALIGN(8);
        _sheap = .;
        . = . + _Heap_Size;
        . = ALIGN(8);
        _eheap = .;
    } > RAM

    _estack = ORIGIN(RAM) + LENGTH(RAM);

    ASSERT(_eheap + _Min_Stack_Size <= _estack, "RAM overflow: heap + main stack do not fit")
}