#include "vgsx.h"
#include <stdint.h>

extern "C" uint32_t m68k_read_memory_8(uint32_t address)
{
    return 0;
}

extern "C" uint32_t m68k_read_memory_16(uint32_t address)
{
    return 0;
}

extern "C" uint32_t m68k_read_memory_32(uint32_t address)
{
    return 0;
}

extern "C" void m68k_write_memory_8(uint32_t address, uint32_t value)
{
}

extern "C" void m68k_write_memory_16(uint32_t address, uint32_t value)
{
}

extern "C" void m68k_write_memory_32(uint32_t address, uint32_t value)
{
}
