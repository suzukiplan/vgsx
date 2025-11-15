MEMORY
{
    ROM (rx)  : ORIGIN = 0x000000, LENGTH = 12288K
    RAM (rwx) : ORIGIN = 0xF00000, LENGTH = 1024K
}

SECTIONS
{
    /* ------------------------------
       68k Exception Vector Table
       ------------------------------ */
    .vectors ORIGIN(ROM) :
    {
        KEEP(*(.vectors))
    } > ROM

    /* ------------------------------
       Program code
       ------------------------------ */
    .text :
    {
        *(.text)
        *(.text.*)
    } > ROM

    /* ------------------------------
       Initialized data (copied to RAM)
       ------------------------------ */
    .data :
    {
        *(.data)
        *(.data.*)
    } > RAM AT > ROM

    /* ------------------------------
       Zero-initialized data
       ------------------------------ */
    .bss (NOLOAD) :
    {
        *(.bss)
        *(.bss.*)
        *(COMMON)
    } > RAM
}
