/**
 * VGS-X Emulator Core Module
 * The MIT License (MIT)
 *
 * Copyright (c) 2025 Yoji Suzuki.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdarg.h>
#include "vgsx.h"
#include "m68k.h"

VGSX vgsx;

typedef struct {
    uint8_t e_ident[16];
    uint16_t e_type;
    uint16_t e_machine;
    uint32_t e_version;
    uint32_t e_entry;
    uint32_t e_phoff;
    uint32_t e_shoff;
    uint32_t e_flags;
    uint16_t e_ehsize;
    uint16_t e_phentsize;
    uint16_t e_phnum;
    uint16_t e_shentsize;
    uint16_t e_shnum;
    uint16_t e_shstrndx;
} Elf32_Ehdr;

typedef struct {
    uint32_t p_type;
    uint32_t p_offset;
    uint32_t p_vaddr;
    uint32_t p_paddr;
    uint32_t p_filesz;
    uint32_t p_memsz;
    uint32_t p_flags;
    uint32_t p_align;
} Elf32_Phdr;

typedef struct {
    uint32_t sh_name;
    uint32_t sh_type;
    uint32_t sh_flags;
    uint32_t sh_addr;
    uint32_t sh_offset;
    uint32_t sh_size;
    uint32_t sh_link;
    uint32_t sh_info;
    uint32_t sh_addralign;
    uint32_t sh_entsize;
} Elf32_Shdr;

static uint16_t b2h16(uint16_t n)
{
    uint8_t* ptr = (uint8_t*)&n;
    uint16_t result = ptr[0];
    result <<= 8;
    result |= ptr[1];
    return result;
}

static uint32_t b2h32(uint32_t n)
{
    uint8_t* ptr = (uint8_t*)&n;
    uint32_t result = ptr[0];
    result <<= 8;
    result |= ptr[1];
    result <<= 8;
    result |= ptr[2];
    result <<= 8;
    result |= ptr[3];
    return result;
}

static void loadElfHeader(Elf32_Ehdr* header, const void* prg)
{
    memcpy(header, prg, sizeof(Elf32_Ehdr));
    header->e_type = b2h16(header->e_type);
    header->e_machine = b2h16(header->e_machine);
    header->e_version = b2h32(header->e_version);
    header->e_entry = b2h32(header->e_entry);
    header->e_phoff = b2h32(header->e_phoff);
    header->e_shoff = b2h32(header->e_shoff);
    header->e_flags = b2h32(header->e_flags);
    header->e_ehsize = b2h16(header->e_ehsize);
    header->e_phentsize = b2h16(header->e_phentsize);
    header->e_phnum = b2h16(header->e_phnum);
    header->e_shentsize = b2h16(header->e_shentsize);
    header->e_shnum = b2h16(header->e_shnum);
    header->e_shstrndx = b2h16(header->e_shstrndx);
}

extern "C" uint32_t m68k_read_memory_8(uint32_t address)
{
    if (address < 0xC00000) {
        return address < vgsx.context.programSize ? vgsx.context.program[address] : 0xFF;
    } else if (0xF00000 <= address) {
        return vgsx.context.ram[address & 0xFFFFF];
    }
    return 0xFF;
}

extern "C" uint32_t m68k_read_memory_16(uint32_t address)
{
    uint16_t result = m68k_read_memory_8(address);
    result <<= 8;
    result |= m68k_read_memory_8(address + 1);
    return result;
}

extern "C" uint32_t m68k_read_memory_32(uint32_t address)
{
    if (0xC00000 <= address && address < 0xE00000) {
        return vgsx.vdp.read(address);
    } else if (0xF00000 <= address || address < 0xC00000) {
        uint32_t result = m68k_read_memory_8(address);
        result <<= 8;
        result |= m68k_read_memory_8(address + 1);
        result <<= 8;
        result |= m68k_read_memory_8(address + 2);
        result <<= 8;
        result |= m68k_read_memory_8(address + 3);
        return result;
    } else if (0xE00000 <= address) {
        return vgsx.inPort(address);
    }
    return 0xFFFFFFFF;
}

extern "C" uint32_t m68k_read_disassembler_8(uint32_t address) { return m68k_read_memory_8(address); }
extern "C" uint32_t m68k_read_disassembler_16(uint32_t address) { return m68k_read_memory_16(address); }
extern "C" uint32_t m68k_read_disassembler_32(uint32_t address) { return m68k_read_memory_32(address); }

extern "C" void m68k_write_memory_8(uint32_t address, uint32_t value)
{
    if (0xF00000 <= address) {
        vgsx.context.ram[address & 0xFFFFF] = value & 0xFF;
    }
}

extern "C" void m68k_write_memory_16(uint32_t address, uint32_t value)
{
    m68k_write_memory_8(address, (value & 0xFF00) >> 8);
    m68k_write_memory_8(address + 1, value & 0xFF);
}

extern "C" void m68k_write_memory_32(uint32_t address, uint32_t value)
{
    if (0xF00000 <= address) {
        m68k_write_memory_8(address, (value & 0xFF000000) >> 24);
        m68k_write_memory_8(address + 1, (value & 0xFF0000) >> 16);
        m68k_write_memory_8(address + 2, (value & 0xFF00) >> 8);
        m68k_write_memory_8(address + 3, value & 0xFF);
    } else if (0xE00000 <= address) {
        vgsx.outPort(address, value);
    } else if (0xC00000 <= address) {
        vgsx.vdp.write(address, value);
    }
}

VGSX::VGSX()
{
    m68k_set_cpu_type(M68K_CPU_TYPE_68030);
    m68k_init();
    this->reset();
}

VGSX::~VGSX()
{
}

void VGSX::setLastError(const char* format, ...)
{
    va_list args;
    va_start(args, format);
    vsnprintf(this->lastError, sizeof(this->lastError), format, args);
    va_end(args);
}

bool VGSX::loadPattern(uint16_t index, const void* data, size_t size)
{
    if (!data) {
        this->setLastError("No data.");
        return false;
    }
    if (size < 32 || (size & 0x1F)) {
        this->setLastError("Invalid data size.");
        return false;
    }
    const uint8_t* ptr = (const uint8_t*)data;
    while (0 < size) {
        memcpy(&this->vdp.context.ptn[index++], ptr, 32);
        ptr += 32;
        size -= 32;
    }
    return true;
}

bool VGSX::loadPalette(const void* data, size_t size)
{
    if (!data) {
        this->setLastError("No data.");
        return false;
    }
    if (size < 4 || 1024 < size || (size & 3)) {
        this->setLastError("Invalid data size.");
        return false;
    }
    int pindex = 0;
    int cindex = 0;
    const uint8_t* ptr = (const uint8_t*)data;
    while (0 < size) {
        memcpy(&this->vdp.context.palette[pindex][cindex], ptr, 4);
        cindex++;
        cindex &= 0x0F;
        if (0 == cindex) {
            pindex++;
            pindex &= 0x0F;
        }
        ptr += 4;
        size -= 4;
    }
    return true;
}

bool VGSX::loadProgram(const void* data, size_t size)
{
    if (!data) {
        this->setLastError("No data.");
        return false;
    }
    if (size < 0x2000) {
        this->setLastError("Invalid program size.");
        return false;
    }

    // check ELF header
    Elf32_Ehdr header;
    loadElfHeader(&header, data);

    // validate ident
    if (header.e_ident[0] != 0x7F ||
        header.e_ident[1] != 'E' ||
        header.e_ident[2] != 'L' ||
        header.e_ident[3] != 'F') {
        this->setLastError("Invalid ELF header.");
        return false;
    }

    // Check ELF32
    if (header.e_ident[4] != 1) {
        this->setLastError("Unsupported ELF type.");
        return false;
    }

    // Check Endian (Big)
    if (header.e_ident[5] != 2) {
        this->setLastError("Unsupported endian model.");
        return false;
    }

    // Check EXEC
    if (header.e_type != 2) {
        this->setLastError("Unsupported execution mode.");
        return false;
    }

    // Check MC68000
    if (header.e_machine != 0x0004) {
        this->setLastError("Unsupported machine model.");
        return false;
    }

    this->context.elf = (const uint8_t*)data;
    this->context.elfSize = size;
    this->reset();
    return true;
}

void VGSX::reset(void)
{
    m68k_pulse_reset();
    m68k_set_reg(M68K_REG_SP, 0);
    this->detectReferVSync = false;
    this->context.program = NULL;
    this->context.programSize = 0;

    if (!this->context.elf) {
        return;
    }

    // Load ELF Header
    Elf32_Ehdr eh;
    loadElfHeader(&eh, this->context.elf);
    printf("M68K_REG_PC = 0x%06X\n", eh.e_entry);
    m68k_set_reg(M68K_REG_PC, eh.e_entry);

    // Search an Executable Code
    for (uint32_t i = 0, off = eh.e_phoff; i < eh.e_phnum; i++, off += eh.e_phentsize) {
        Elf32_Phdr ph;
        memcpy(&ph, &this->context.elf[off], eh.e_phentsize);
        ph.p_type = b2h32(ph.p_type);
        ph.p_offset = b2h32(ph.p_offset);
        ph.p_vaddr = b2h32(ph.p_vaddr);
        ph.p_paddr = b2h32(ph.p_paddr);
        ph.p_filesz = b2h32(ph.p_filesz);
        ph.p_memsz = b2h32(ph.p_memsz);
        ph.p_flags = b2h32(ph.p_flags);
        ph.p_align = b2h32(ph.p_align);
        printf(".program:%X offset=%d, va=0x%06X, pa=0x%06X, fs=%d, ms=%d, flg=%d\n",
               ph.p_type,
               ph.p_offset,
               ph.p_vaddr,
               ph.p_paddr,
               ph.p_filesz,
               ph.p_memsz,
               ph.p_flags);
        if (ph.p_type == 1) {
            if (0 == ph.p_paddr && (ph.p_flags & 0x01)) {
                this->context.program = &this->context.elf[ph.p_offset];
                this->context.programSize = ph.p_memsz;
            }
        }
    }

    // Reset RAM
    memset(this->context.ram, 0xFF, sizeof(this->context.ram));
    // ELF section data relocate to RAM from ROM
    for (uint32_t i = 0, off = eh.e_shoff; i < eh.e_shnum; i++, off += eh.e_shentsize) {
        Elf32_Shdr sh;
        memcpy(&sh, &this->context.elf[off], eh.e_shentsize);
        sh.sh_name = b2h32(sh.sh_name);
        sh.sh_type = b2h32(sh.sh_type);
        sh.sh_flags = b2h32(sh.sh_flags);
        sh.sh_addr = b2h32(sh.sh_addr);
        sh.sh_offset = b2h32(sh.sh_offset);
        sh.sh_size = b2h32(sh.sh_size);
        sh.sh_link = b2h32(sh.sh_link);
        sh.sh_info = b2h32(sh.sh_info);
        if (0xF00000 <= sh.sh_addr) {
            memcpy(&this->context.ram[sh.sh_addr & 0xFFFFF],
                   &this->context.elf[sh.sh_offset],
                   sh.sh_size);
        }
        printf(".section:%X flags=%d, addr=0x%06X, offset=0x%06X, size=%d, link=%d, info=%d\n",
               sh.sh_type,
               sh.sh_flags,
               sh.sh_addr,
               sh.sh_offset,
               sh.sh_size,
               sh.sh_link,
               sh.sh_info);
    }
}

void VGSX::tick(void)
{
    this->detectReferVSync = false;
    while (!this->detectReferVSync) {
        m68k_execute(4);
    }
    this->vdp.render();
}

uint32_t VGSX::inPort(uint32_t address)
{
    switch (address) {
        case 0xE00000: // V-SYNC
            this->detectReferVSync = true;
            return 1;
    }
    return 0xFFFFFFFF;
}

void VGSX::outPort(uint32_t address, uint32_t value)
{
    switch (address) {
        case 0xE00000: // Console Output
            fputc(value, stdout);
            return;
    }
}
