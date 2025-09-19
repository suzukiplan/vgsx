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
#include "vgmrender.hpp"

VGSX vgsx;

extern "C" {
extern const unsigned short vgs0_rand16[65536];
};

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
    memset(&this->context, 0, sizeof(this->context));
    memset(&this->key, 0, sizeof(this->key));
    m68k_set_cpu_type(M68K_CPU_TYPE_68030);
    m68k_init();
    this->reset();
}

VGSX::~VGSX()
{
    if (this->vgmHelper) {
        delete (VgmHelper*)this->vgmHelper;
    }
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

bool VGSX::loadVgm(uint16_t index, const void* data, size_t size)
{
    this->context.vgmData[index].data = (const uint8_t*)data;
    this->context.vgmData[index].size = size;
    return true;
}

bool VGSX::loadWav(uint8_t index, const void* data, size_t size)
{
    const uint8_t* wav = (const uint8_t*)data;
    int n;
    unsigned short s;

    if (0 != memcmp(wav, "RIFF", 4)) {
        this->setLastError("Invalid wav format: RIFF not exist");
        return false;
    }
    wav += 4;
    size -= 4;

    memcpy(&n, wav, 4);
    if (n != size - 4) {
        this->setLastError("Invalid wav format: SIZE");
        return false;
    }
    wav += 4;
    size -= 4;

    if (0 != memcmp(wav, "WAVE", 4)) {
        this->setLastError("Invalid wav format: not WAVE format");
        return false;
    }
    wav += 4;
    size -= 4;

    short ch = 0;
    int rate = 0;
    int bps = 0;
    short bs = 0;
    short bits = 0;

    // parse and check the chunks
    while (1) {
        if (0 == memcmp(wav, "fmt ", 4)) {
            wav += 4;
            size -= 4;
            memcpy(&n, wav, 4);
            if (n != 16) {
                this->setLastError("Invalid wav format: Unsupported format (%d)", n);
                return false;
            }
            wav += 4;
            size -= 4;

            memcpy(&s, wav, 2);
            if (s != 0x0001) {
                this->setLastError("Invalid wav format: Unsupported compress type (%d)", s);
                return false;
            }
            wav += 2;
            size -= 2;

            memcpy(&ch, wav, 2);
            wav += 2;
            size -= 2;

            memcpy(&rate, wav, 4);
            wav += 4;
            size -= 4;

            memcpy(&bps, wav, 4);
            wav += 4;
            size -= 4;

            memcpy(&bs, wav, 2);
            wav += 2;
            size -= 2;

            memcpy(&bits, wav, 2);
            wav += 2;
            size -= 2;
        } else if (0 == memcmp(wav, "LIST", 4)) {
            wav += 4;
            size -= 4;
            memcpy(&n, wav, 4);
            wav += 4 + n;
            size -= 4 + n;
        } else if (0 == memcmp(wav, "data", 4)) {
            wav += 4;
            size -= 4;
            break;
        } else {
            this->setLastError("Invalid wav format: Unsupported chunk: %c%c%c%c", wav[0], wav[1], wav[2], wav[3]);
            return false;
        }
    }

    printf("- PCM Format: %dHz %dbits %dch (%d bytes/sec, %d bytes/sample)\n", rate, bits, ch, bps, bs);
    if (0 == rate) {
        this->setLastError("Invalid wav format: fmt chunk not found");
        return false;
    } else if (rate != 44100 || bits != 16 || ch != 2) {
        this->setLastError("Invalid wav format: Unsupported sampling format (44100Hz/16bits/2ch only)");
        return false;
    }

    // validate data chunk size
    memcpy(&n, wav, 4);
    if (n != size - 4) {
        this->setLastError("Invalid wav format: invalid sub chunk size");
        return false;
    }
    size -= 4;
    this->context.sfxData[index].data = (const int16_t*)wav;
    this->context.sfxData[index].count = size / 2;
    return true;
}

void VGSX::reset(void)
{
    m68k_pulse_reset();
    m68k_set_reg(M68K_REG_SP, 0);
    this->detectReferVSync = false;
    this->exitFlag = false;
    this->exitCode = 0;
    this->context.program = NULL;
    this->context.programSize = 0;
    this->context.randomIndex = 0;
    this->context.frameClocks = 0;
    this->vdp.reset();
    if (this->vgmHelper) {
        delete (VgmHelper*)this->vgmHelper;
        this->vgmHelper = nullptr;
    }
    for (int i = 0; i < 0x100; i++) {
        this->context.sfxData[i].play = false;
    }
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
    this->context.frameClocks = 0;
    while (!this->detectReferVSync && !this->exitFlag) {
        m68k_execute(4);
        this->context.frameClocks += 4;
    }
    this->vdp.render();
}

void VGSX::tickSound(int16_t* buf, int samples)
{
    memset(buf, 0, samples * 2);
    auto helper = (VgmHelper*)this->vgmHelper;
    if (helper) {
        helper->render(buf, samples);
    }
    for (int i = 0; i < 0x100; i++) {
        if (this->context.sfxData[i].play) {
            for (int j = 0; j < samples; j++) {
                if (this->context.sfxData[i].index < this->context.sfxData[i].count) {
                    int w = buf[j];
                    w += this->context.sfxData[i].data[this->context.sfxData[i].index];
                    if (32767 < w) {
                        w = 32767;
                    } else if (w < -32768) {
                        w = -32768;
                    }
                    buf[j] = w;
                    this->context.sfxData[i].index++;
                } else {
                    this->context.sfxData[i].play = false;
                    break;
                }
            }
        }
    }
}

uint32_t VGSX::inPort(uint32_t address)
{
    switch (address) {
        case 0xE00000: // V-SYNC
            this->detectReferVSync = true;
            return 1;
        case 0xE00004: // Random
            this->context.randomIndex++;
            this->context.randomIndex &= 0xFFFF;
            return vgs0_rand16[this->context.randomIndex];
        case 0xE00014: return this->dmaSearch();
        case 0xE20000: return this->key.up;
        case 0xE20004: return this->key.down;
        case 0xE20008: return this->key.left;
        case 0xE2000C: return this->key.right;
        case 0xE20010: return this->key.a;
        case 0xE20014: return this->key.b;
        case 0xE20018: return this->key.x;
        case 0xE2001C: return this->key.y;
        case 0xE20020: return this->key.start;
        case 0xE20024: return this->key.axisX;
        case 0xE20028: return this->key.axisY;
    }
    return 0xFFFFFFFF;
}

void VGSX::outPort(uint32_t address, uint32_t value)
{
    switch (address) {
        case 0xE00000: // Console Output
            fputc(value, stdout);
            return;
        case 0xE00004: // Setup Random
            this->context.randomIndex = (int)value;
            return;
        case 0xE00008: // DMA (Source)
            this->context.dmaSource = value;
            return;
        case 0xE0000C: // DMA (Destination)
            this->context.dmaDestination = value;
            return;
        case 0xE00010: // DMA (Argument)
            this->context.dmaArgument = value;
            return;
        case 0xE00014: // DMA (Execute)
            switch (value) {
                case 0: this->dmaMemcpy(); break;
                case 1: this->dmaMemset(); break;
            }
            return;
        case 0xE01000: // Play VGM
            value &= 0xFFFF;
            if (this->context.vgmData[value].data) {
                auto helper = (VgmHelper*)this->vgmHelper;
                if (helper) {
                    delete helper;
                }
                helper = new VgmHelper(this->context.vgmData[value].data, this->context.vgmData[value].size);
                this->vgmHelper = helper;
            }
            return;
        case 0xE01100: // Play SFX
            value &= 0xFF;
            if (this->context.sfxData[value].data) {
                this->context.sfxData[value].index = 0;
                this->context.sfxData[value].play = true;
            }
            return;
        case 0xE7FFFC: // Exit
            this->exitFlag = true;
            this->exitCode = (int32_t)value;
            return;
    }
}

void VGSX::dmaMemcpy()
{
    const uint32_t size = this->context.dmaArgument;
    uint32_t destination = this->context.dmaDestination & 0x00FFFFFF;
    uint32_t source = this->context.dmaSource & 0x00FFFFFF;
    // validate destination
    if (0xF00000 <= destination && destination + size <= 0xFFFFFF) {
        // validate source
        if (source < this->context.programSize) {
            if (source + size <= this->context.programSize) {
                // Execute copy from ROM to RAM
                memcpy(&this->context.ram[destination & 0x0FFFFF],
                       &this->context.program[source],
                       size);
                return;
            }
        } else if (0xF00000 <= source) {
            if (source + size <= 0xFFFFFF) {
                // Execute copy from RAM to RAM
                memmove(&this->context.ram[destination & 0x0FFFFF],
                        &this->context.ram[source & 0x0FFFFF],
                        size);
                return;
            }
        }
    }
    printf("warning: Ignored an invalid DMA_copy(0x%06X, 0x%06X, %u)\n", destination, source, size);
}

void VGSX::dmaMemset()
{
    uint32_t destination = this->context.dmaDestination & 0x00FFFFFF;
    const uint8_t c = this->context.dmaSource & 0xFF;
    const uint32_t size = this->context.dmaArgument;
    // validate destination
    if (0xF00000 <= destination && destination + size <= 0xFFFFFF) {
        // Bulk set to RAM
        memset(&this->context.ram[destination & 0x0FFFFF], c, size);
        return;
    }
    printf("warning: Ignored an invalid DMA_set(0x%06X, 0x%02X, %u)\n", destination, c, size);
}

uint32_t VGSX::dmaSearch()
{
    uint8_t search = this->context.dmaArgument & 0xFF;
    uint32_t ptr = this->context.dmaSource & 0x00FFFFFF;
    // validate source
    if (ptr < this->context.programSize) {
        // Search from ROM
        for (; ptr < this->context.programSize; ptr++) {
            if (this->context.program[ptr] == search) {
                return ptr; // found
            }
        }
        return 0; // not found
    } else if (0xF00000 <= ptr && ptr < 0xFFFFFF) {
        // Search from RAM
        for (; ptr < 0xFFFFFF; ptr++) {
            if (this->context.ram[ptr] == search) {
                return ptr; // found
            }
        }
        return 0; // not found
    }
    printf("warning: Ignored an invalid DMA_search(0x%06X, 0x%02X)\n", ptr, search);
    return 0;
}
