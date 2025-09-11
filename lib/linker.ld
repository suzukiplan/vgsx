MEMORY
{
    ROM (rx) : ORIGIN = 0x000000, LENGTH = 12288K
    RAM (rwx) : ORIGIN = 0xF00000, LENGTH = 1024K
}

SECTIONS
{
    .text :
    {
        *(.text)
    } > ROM

    .data :
    {
        *(.data)
    } > RAM

    .bss :
    {
        *(.bss)
    } > RAM
}
