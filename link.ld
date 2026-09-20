MEMORY
{
    /* SRAM A1: 0x00000000 - 0x00007FFF (32KB)
       Код размещаем начиная с 0x2000, чтобы не трогать
       FEL scratchpad BROM (0x7E00) и служебные области. */
    SRAM (rwx) : ORIGIN = 0x00002000, LENGTH = 0x6000
}

SECTIONS
{
    . = ORIGIN(SRAM);

    .text : {
        KEEP(*(.text.vector))
        *(.text*)
    } > SRAM

    .rodata : {
        *(.rodata*)
    } > SRAM

    .data : {
        *(.data*)
    } > SRAM

    .bss (NOLOAD) : {
        *(.bss*)
        *(COMMON)
    } > SRAM

    /* Стек: верхушка от 0x4000, растёт вниз, не пересекается с кодами */
    stack_top = 0x00004000;
}