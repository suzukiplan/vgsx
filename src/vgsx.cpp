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
#include "vgs_io.h"

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
        return address < vgsx.ctx.programSize ? vgsx.ctx.program[address] : 0xFF;
    } else if (0xF00000 <= address) {
        return vgsx.ctx.ram[address & 0xFFFFF];
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
        vgsx.ctx.ram[address & 0xFFFFF] = value & 0xFF;
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
    this->gamepadType = GamepadType::Keyboard;
    this->logCallback = nullptr;
    memset(&this->ctx, 0, sizeof(this->ctx));
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

void VGSX::putlog(LogLevel level, const char* format, ...)
{
    if (this->logCallback) {
        char buf[1024];
        va_list args;
        va_start(args, format);
        vsnprintf(buf, sizeof(buf), format, args);
        va_end(args);
        this->logCallback(level, buf);
    }
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
        memcpy(&this->vdp.ctx.ptn[index++], ptr, 32);
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
        memcpy(&this->vdp.ctx.palette[pindex][cindex], ptr, 4);
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

    this->ctx.elf = (const uint8_t*)data;
    this->ctx.elfSize = size;
    this->reset();
    return true;
}

bool VGSX::loadVgm(uint16_t index, const void* data, size_t size)
{
    this->ctx.vgmData[index].data = (const uint8_t*)data;
    this->ctx.vgmData[index].size = size;
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

    putlog(LogLevel::I, "- PCM Format: %dHz %dbits %dch (%d bytes/sec, %d bytes/sample)", rate, bits, ch, bps, bs);
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
    this->ctx.sfxData[index].data = (const int16_t*)wav;
    this->ctx.sfxData[index].count = size / 2;
    return true;
}

void VGSX::reset(void)
{
    m68k_pulse_reset();
    m68k_set_reg(M68K_REG_SP, 0);
    this->detectReferVSync = false;
    this->exitFlag = false;
    this->exitCode = 0;
    this->ctx.program = NULL;
    this->ctx.programSize = 0;
    this->ctx.randomIndex = 0;
    this->ctx.frameClocks = 0;
    this->vdp.reset();
    if (this->vgmHelper) {
        delete (VgmHelper*)this->vgmHelper;
        this->vgmHelper = nullptr;
    }
    for (int i = 0; i < 0x100; i++) {
        this->ctx.sfxData[i].play = false;
    }
    if (!this->ctx.elf) {
        return;
    }

    // Load ELF Header
    Elf32_Ehdr eh;
    loadElfHeader(&eh, this->ctx.elf);
    putlog(LogLevel::I, "M68K_REG_PC = 0x%06X", eh.e_entry);
    m68k_set_reg(M68K_REG_PC, eh.e_entry);

    // Search an Executable Code
    for (uint32_t i = 0, off = eh.e_phoff; i < eh.e_phnum; i++, off += eh.e_phentsize) {
        Elf32_Phdr ph;
        memcpy(&ph, &this->ctx.elf[off], eh.e_phentsize);
        ph.p_type = b2h32(ph.p_type);
        ph.p_offset = b2h32(ph.p_offset);
        ph.p_vaddr = b2h32(ph.p_vaddr);
        ph.p_paddr = b2h32(ph.p_paddr);
        ph.p_filesz = b2h32(ph.p_filesz);
        ph.p_memsz = b2h32(ph.p_memsz);
        ph.p_flags = b2h32(ph.p_flags);
        ph.p_align = b2h32(ph.p_align);
        putlog(LogLevel::I, ".program:%X offset=%d, va=0x%06X, pa=0x%06X, fs=%d, ms=%d, flg=%d",
               ph.p_type,
               ph.p_offset,
               ph.p_vaddr,
               ph.p_paddr,
               ph.p_filesz,
               ph.p_memsz,
               ph.p_flags);
        if (ph.p_type == 1) {
            if (0 == ph.p_paddr && (ph.p_flags & 0x01)) {
                this->ctx.program = &this->ctx.elf[ph.p_offset];
                this->ctx.programSize = ph.p_memsz;
            }
        }
    }

    // Reset RAM
    memset(this->ctx.ram, 0xFF, sizeof(this->ctx.ram));
    // ELF section data relocate to RAM from ROM
    for (uint32_t i = 0, off = eh.e_shoff; i < eh.e_shnum; i++, off += eh.e_shentsize) {
        Elf32_Shdr sh;
        memcpy(&sh, &this->ctx.elf[off], eh.e_shentsize);
        sh.sh_name = b2h32(sh.sh_name);
        sh.sh_type = b2h32(sh.sh_type);
        sh.sh_flags = b2h32(sh.sh_flags);
        sh.sh_addr = b2h32(sh.sh_addr);
        sh.sh_offset = b2h32(sh.sh_offset);
        sh.sh_size = b2h32(sh.sh_size);
        sh.sh_link = b2h32(sh.sh_link);
        sh.sh_info = b2h32(sh.sh_info);
        if (0xF00000 <= sh.sh_addr) {
            memcpy(&this->ctx.ram[sh.sh_addr & 0xFFFFF],
                   &this->ctx.elf[sh.sh_offset],
                   sh.sh_size);
        }
        putlog(LogLevel::I, ".section:%X flags=%d, addr=0x%06X, offset=0x%06X, size=%d, link=%d, info=%d",
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
    this->ctx.frameClocks = 0;
    while (!this->detectReferVSync && !this->exitFlag) {
        m68k_execute(4);
        this->ctx.frameClocks += 4;
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
        if (this->ctx.sfxData[i].play) {
            for (int j = 0; j < samples; j++) {
                if (this->ctx.sfxData[i].index < this->ctx.sfxData[i].count) {
                    int w = buf[j];
                    w += this->ctx.sfxData[i].data[this->ctx.sfxData[i].index];
                    if (32767 < w) {
                        w = 32767;
                    } else if (w < -32768) {
                        w = -32768;
                    }
                    buf[j] = w;
                    this->ctx.sfxData[i].index++;
                } else {
                    this->ctx.sfxData[i].play = false;
                    break;
                }
            }
        }
    }
}

uint32_t VGSX::inPort(uint32_t address)
{
    switch (address) {
        case VGS_ADDR_VSYNC: // V-SYNC
            this->detectReferVSync = true;
            return 1;
        case VGS_ADDR_RANDOM: // Random
            this->ctx.randomIndex++;
            this->ctx.randomIndex &= 0xFFFF;
            return vgs0_rand16[this->ctx.randomIndex];
        case VGS_ADDR_DMA_EXECUTE: return this->dmaSearch();

        case VGS_ADDR_ANGLE_DEGREE: // atan2
            this->ctx.angle.radian = atan2(this->ctx.angle.y2 - this->ctx.angle.y1,
                                           this->ctx.angle.x2 - this->ctx.angle.x1);
            this->ctx.angle.degree = (int32_t)(this->ctx.angle.radian * 180 / M_PI);
            this->ctx.angle.degree %= 360;
            if (this->ctx.angle.degree < 0) {
                this->ctx.angle.degree += 360;
            }
            return this->ctx.angle.degree;
        case VGS_ADDR_ANGLE_SIN: return (int32_t)(sin(this->ctx.angle.radian) * 256);
        case VGS_ADDR_ANGLE_COS: return (int32_t)(cos(this->ctx.angle.radian) * 256);

        case VGS_ADDR_KEY_UP: return this->key.up;
        case VGS_ADDR_KEY_DOWN: return this->key.down;
        case VGS_ADDR_KEY_LEFT: return this->key.left;
        case VGS_ADDR_KEY_RIGHT: return this->key.right;
        case VGS_ADDR_KEY_A: return this->key.a;
        case VGS_ADDR_KEY_B: return this->key.b;
        case VGS_ADDR_KEY_X: return this->key.x;
        case VGS_ADDR_KEY_Y: return this->key.y;
        case VGS_ADDR_KEY_START: return this->key.start;

        case VGS_ADDR_SAVE_EXECUTE: // Execute Load
        {
            this->ctx.save.address &= 0x00FFFFFF;
            if (this->ctx.save.address < 0xF00000) {
                putlog(LogLevel::W, "ignored an invalid load request (addr=0x%X)", this->ctx.save.address);
                return 0;
            }
            FILE* fp = fopen("save.dat", "rb");
            if (!fp) {
                putlog(LogLevel::E, "failed load request (File Not Found!)");
                return 0;
            }
            if (fseek(fp, 0, SEEK_END) < 0) {
                putlog(LogLevel::E, "failed load request (Seek END Failed!)");
                fclose(fp);
                return 0;
            }
            int32_t size = ftell(fp);
            if (size < 1 || 0x100000 < size) {
                putlog(LogLevel::E, "failed load request (Invalid Size: %d)", size);
                fclose(fp);
                return 0;
            }
            if (0x00FFFFFF < this->ctx.save.address + size) {
                putlog(LogLevel::E, "failed load request (Overflow: %X+%d)", this->ctx.save.address, size);
                fclose(fp);
                return 0;
            }
            if (fseek(fp, 0, SEEK_SET) < 0) {
                putlog(LogLevel::E, "failed load request (Seek SET Failed!)");
                fclose(fp);
                return 0;
            }
            if (size != fread(&this->ctx.ram[this->ctx.save.address & 0xFFFFF], 1, size, fp)) {
                putlog(LogLevel::E, "failed load request (Read Failed!)");
                fclose(fp);
                return 0;
            }
            fclose(fp);
            putlog(LogLevel::N, "save.dat read (%u bytes)", size);
            return size;
        }

        case VGS_ADDR_SAVE_CHECK: // Check save.dat size
        {
            FILE* fp = fopen("save.dat", "rb");
            if (!fp) {
                putlog(LogLevel::W, "failed check save.dat size request (File Not Found!)");
                return 0;
            }
            if (fseek(fp, 0, SEEK_END) < 0) {
                putlog(LogLevel::W, "failed check save.dat size request (Seek END Failed!)");
                fclose(fp);
                return 0;
            }
            int32_t size = ftell(fp);
            if (size < 1 || 0x100000 < size) {
                putlog(LogLevel::W, "failed check save.dat size request (Invalid Size: %d)", size);
                fclose(fp);
                return 0;
            }
            fclose(fp);
            return size;
        }

        case VGS_ADDR_SEQ_READ:
            if (this->ctx.sqr.size <= this->ctx.sqr.readOffset) {
                return 0xFFFFFFFF;
            }
            return this->ctx.sqr.buffer[this->ctx.sqr.readOffset++];
    }
    return 0xFFFFFFFF;
}

void VGSX::outPort(uint32_t address, uint32_t value)
{
    switch (address) {
        case VGS_ADDR_CONSOLE: // Console Output
            fputc(value, stdout);
            return;
        case VGS_ADDR_RANDOM: // Setup Random
            this->ctx.randomIndex = (int)value;
            return;
        case VGS_ADDR_DMA_SOURCE: // DMA (Source)
            this->ctx.dma.source = value;
            return;
        case VGS_ADDR_DMA_DESTINATION: // DMA (Destination)
            this->ctx.dma.destination = value;
            return;
        case VGS_ADDR_DMA_ARGUMENT: // DMA (Argument)
            this->ctx.dma.argument = value;
            return;
        case VGS_ADDR_DMA_EXECUTE: // DMA (Execute)
            switch (value) {
                case 0: this->dmaMemcpy(); break;
                case 1: this->dmaMemset(); break;
            }
            return;

        // Angle
        case VGS_ADDR_ANGLE_X1: this->ctx.angle.x1 = (int32_t)value; return;
        case VGS_ADDR_ANGLE_Y1: this->ctx.angle.y1 = (int32_t)value; return;
        case VGS_ADDR_ANGLE_X2: this->ctx.angle.x2 = (int32_t)value; return;
        case VGS_ADDR_ANGLE_Y2: this->ctx.angle.y2 = (int32_t)value; return;
        case VGS_ADDR_ANGLE_DEGREE:
            this->ctx.angle.degree = (int32_t)value;
            this->ctx.angle.degree %= 360;
            if (this->ctx.angle.degree < 0) {
                this->ctx.angle.degree += 360;
            }
            this->ctx.angle.radian = this->ctx.angle.degree * (M_PI / 180.0);
            return;

        case VGS_ADDR_VGM_PLAY: // Play VGM
            value &= 0xFFFF;
            if (this->ctx.vgmData[value].data) {
                auto helper = (VgmHelper*)this->vgmHelper;
                if (helper) {
                    delete helper;
                }
                helper = new VgmHelper(this->ctx.vgmData[value].data, this->ctx.vgmData[value].size, this->logCallback);
                this->vgmHelper = helper;
            }
            return;
        case VGS_ADDR_SFX_PLAY: // Play SFX
            value &= 0xFF;
            if (this->ctx.sfxData[value].data) {
                this->ctx.sfxData[value].index = 0;
                this->ctx.sfxData[value].play = true;
            }
            return;

        case VGS_ADDR_SAVE_ADDRESS: // Save Data
            this->ctx.save.address = value;
            return;
        case VGS_ADDR_SAVE_EXECUTE: // Execute Save
            this->ctx.save.address &= 0x00FFFFFF;
            if (this->ctx.save.address < 0xF00000) {
                putlog(LogLevel::W, "ignored an invalid save request (addr=0x%X)", this->ctx.save.address);
            } else if (0x100000 < value || value < 1) {
                putlog(LogLevel::W, "ignored an invalid save request (size=%u)", value);
            } else if (0x00FFFFFF < this->ctx.save.address + value) {
                putlog(LogLevel::W, "ignored an invalid save request (addr=0x%X+%u)", this->ctx.save.address, value);
            } else {
                FILE* fp = fopen("save.dat", "wb");
                if (!fp) {
                    putlog(LogLevel::E, "save.dat open failed!");
                } else {
                    if (value != fwrite(&this->ctx.ram[this->ctx.save.address & 0xFFFFF], 1, value, fp)) {
                        putlog(LogLevel::E, "save.dat write failed!");
                    } else {
                        putlog(LogLevel::N, "save.dat wrote (%u bytes)", value);
                    }
                    fclose(fp);
                }
            }
            return;

        case VGS_ADDR_SEQ_OPEN_W: // Sequencial Open for Write
            this->ctx.sqw.index = value & 0xFF;
            this->ctx.sqw.size = 0;
            return;

        case VGS_ADDR_SEQ_WRITE: // Write Sequencial Data
            this->ctx.sqw.buffer[this->ctx.sqw.size++] = value & 0xFF;
            this->ctx.sqw.size &= 0xFFFFF;
            return;

        case VGS_ADDR_SEQ_COMMIT: {
            char fname[80];
            snprintf(fname, sizeof(fname), "save%03d.dat", (int)this->ctx.sqw.index);
            FILE* fp = fopen(fname, "wb");
            if (fp) {
                if (this->ctx.sqw.size != fwrite(this->ctx.sqw.buffer, 1, this->ctx.sqw.size, fp)) {
                    putlog(LogLevel::E, "file write error (%s)", fname);
                } else {
                    putlog(LogLevel::N, "%s wrote (%u bytes)", fname, this->ctx.sqw.size);
                }
                fclose(fp);
            }
            return;
        }

        case VGS_ADDR_SEQ_OPEN_R: {
            this->ctx.sqr.index = value & 0xFF;
            char fname[80];
            snprintf(fname, sizeof(fname), "save%03d.dat", (int)this->ctx.sqr.index);
            FILE* fp = fopen(fname, "rb");
            if (fp) {
                this->ctx.sqr.size = fread(this->ctx.sqr.buffer, 1, sizeof(this->ctx.sqr.buffer), fp);
                putlog(LogLevel::N, "%s read (%u bytes)", fname, this->ctx.sqr.size);
                fclose(fp);
            } else {
                putlog(LogLevel::E, "File not found (%s)", fname);
                this->ctx.sqr.size = 0;
            }
            this->ctx.sqr.readOffset = 0;
            return;
        }

        case 0xE7FFFC: // Exit
            this->exitFlag = true;
            this->exitCode = (int32_t)value;
            return;
    }
}

void VGSX::dmaMemcpy()
{
    const uint32_t size = this->ctx.dma.argument;
    uint32_t destination = this->ctx.dma.destination & 0x00FFFFFF;
    uint32_t source = this->ctx.dma.source & 0x00FFFFFF;
    // validate destination
    if (0xF00000 <= destination && destination + size <= 0xFFFFFF) {
        // validate source
        if (source < this->ctx.programSize) {
            if (source + size <= this->ctx.programSize) {
                // Execute copy from ROM to RAM
                memcpy(&this->ctx.ram[destination & 0x0FFFFF],
                       &this->ctx.program[source],
                       size);
                return;
            }
        } else if (0xF00000 <= source) {
            if (source + size <= 0xFFFFFF) {
                // Execute copy from RAM to RAM
                memmove(&this->ctx.ram[destination & 0x0FFFFF],
                        &this->ctx.ram[source & 0x0FFFFF],
                        size);
                return;
            }
        }
    }
    putlog(LogLevel::W, "Ignored an invalid DMA_copy(0x%06X, 0x%06X, %u)", destination, source, size);
}

void VGSX::dmaMemset()
{
    uint32_t destination = this->ctx.dma.destination & 0x00FFFFFF;
    const uint8_t c = this->ctx.dma.source & 0xFF;
    const uint32_t size = this->ctx.dma.argument;
    // validate destination
    if (0xF00000 <= destination && destination + size <= 0xFFFFFF) {
        // Bulk set to RAM
        memset(&this->ctx.ram[destination & 0x0FFFFF], c, size);
        return;
    }
    putlog(LogLevel::W, "Ignored an invalid DMA_set(0x%06X, 0x%02X, %u)", destination, c, size);
}

uint32_t VGSX::dmaSearch()
{
    uint8_t search = this->ctx.dma.argument & 0xFF;
    uint32_t ptr = this->ctx.dma.source & 0x00FFFFFF;
    // validate source
    if (ptr < this->ctx.programSize) {
        // Search from ROM
        for (; ptr < this->ctx.programSize; ptr++) {
            if (this->ctx.program[ptr] == search) {
                return ptr; // found
            }
        }
        return 0; // not found
    } else if (0xF00000 <= ptr && ptr < 0xFFFFFF) {
        // Search from RAM
        for (; ptr < 0xFFFFFF; ptr++) {
            if (this->ctx.ram[ptr] == search) {
                return ptr; // found
            }
        }
        return 0; // not found
    }
    putlog(LogLevel::W, "Ignored an invalid DMA_search(0x%06X, 0x%02X)", ptr, search);
    return 0;
}

VGSX::GamepadKeyName VGSX::getKeyNameA()
{
    switch (this->gamepadType) {
        case GamepadType::Keyboard: return GamepadKeyName::Z;
        case GamepadType::XBOX: return GamepadKeyName::A;
        case GamepadType::NintendoSwitch: return GamepadKeyName::B;
        case GamepadType::PlayStation: return GamepadKeyName::Cross;
        default: return GamepadKeyName::Unknown;
    }
}

VGSX::GamepadKeyName VGSX::getKeyNameB()
{
    switch (this->gamepadType) {
        case GamepadType::Keyboard: return GamepadKeyName::X;
        case GamepadType::XBOX: return GamepadKeyName::B;
        case GamepadType::NintendoSwitch: return GamepadKeyName::A;
        case GamepadType::PlayStation: return GamepadKeyName::Circle;
        default: return GamepadKeyName::Unknown;
    }
}

VGSX::GamepadKeyName VGSX::getKeyNameX()
{
    switch (this->gamepadType) {
        case GamepadType::Keyboard: return GamepadKeyName::A;
        case GamepadType::XBOX: return GamepadKeyName::X;
        case GamepadType::NintendoSwitch: return GamepadKeyName::Y;
        case GamepadType::PlayStation: return GamepadKeyName::Square;
        default: return GamepadKeyName::Unknown;
    }
}

VGSX::GamepadKeyName VGSX::getKeyNameY()
{
    switch (this->gamepadType) {
        case GamepadType::Keyboard: return GamepadKeyName::S;
        case GamepadType::XBOX: return GamepadKeyName::Y;
        case GamepadType::NintendoSwitch: return GamepadKeyName::X;
        case GamepadType::PlayStation: return GamepadKeyName::Triangle;
        default: return GamepadKeyName::Unknown;
    }
}

VGSX::GamepadKeyName VGSX::getKeyNameStart()
{
    switch (this->gamepadType) {
        case GamepadType::Keyboard: return GamepadKeyName::Space;
        case GamepadType::XBOX: return GamepadKeyName::Start;
        case GamepadType::NintendoSwitch: return GamepadKeyName::Plus;
        case GamepadType::PlayStation: return GamepadKeyName::Options;
        default: return GamepadKeyName::Unknown;
    }
}
