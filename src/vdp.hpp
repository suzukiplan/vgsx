/**
 * VGS-X Video Display Processor
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
#pragma once
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <ctype.h>

#define VDP_BG_NUM 4   /* Number of the BG plan */
#define VDP_WIDTH 320  /* Width of the Screen */
#define VDP_HEIGHT 200 /* Height of the Screen */
#define VDP_DISPLAY_SCALE 2
#define VDP_DISPLAY_WIDTH (VDP_WIDTH * VDP_DISPLAY_SCALE)
#define VDP_DISPLAY_HEIGHT (VDP_HEIGHT * VDP_DISPLAY_SCALE)
#define VDP_DISPLAY_PIXELS (VDP_DISPLAY_WIDTH * VDP_DISPLAY_HEIGHT)

extern "C" {
extern const int vgsx_sin[360];
extern const int vgsx_cos[360];
extern const uint8_t k8x12_jisx0201[3072];   // 12x256
extern const uint8_t k8x12_jisx0208[106032]; // 12x8836
};

static inline int range(int n, int min, int max)
{
    if (n < min) {
        n = min;
    }
    if (max < n) {
        n = max;
    }
    return n;
}

class VDP;
static inline void graphicDrawPixel(VDP* vdp);
static inline void graphicDrawLine(VDP* vdp);
static inline void graphicDrawBox(VDP* vdp);
static inline void graphicDrawBoxFill(VDP* vdp);
static inline void graphicDrawCharacter(VDP* vdp);
static inline void graphicDrawJisX0201(VDP* vdp);
static inline void graphicDrawJisX0208(VDP* vdp);
static inline void graphicDrawClear(VDP* vdp);
static inline void graphicDrawWindow(VDP* vdp);

class VDP
{
  private:
    const uint8_t* cpu_ram;
    class PatternRom
    {
      public:
        int index;
        const uint8_t* ptn;
        int size;

        PatternRom(int index, const uint8_t* ptn, int size)
        {
            this->index = index;
            this->ptn = ptn;
            this->size = size;
        }
    };

    struct RomData {
        std::vector<PatternRom*> ptn;
        const uint8_t* pal;
        size_t palSize;
    } rom;

    void resetPattern()
    {
        for (auto ptn : this->rom.ptn) {
            int index = ptn->index;
            const uint8_t* ptr = ptn->ptn;
            int size = ptn->size;
            while (0 < size) {
                memcpy(&this->ctx.ptn[index++], ptr, 32);
                ptr += 32;
                size -= 32;
            }
        }
    }

    void resetPalette()
    {
        const uint8_t* ptr = this->rom.pal;
        int size = (int)this->rom.palSize;
        int pindex = 0;
        int cindex = 0;
        while (0 < size) {
            memcpy(&this->ctx.palette[pindex][cindex], ptr, 4);
            cindex++;
            cindex &= 0x0F;
            if (0 == cindex) {
                pindex++;
                pindex &= 0x0F;
            }
            ptr += 4;
            size -= 4;
        }
    }

  public:
    typedef struct {
        uint32_t skip;                // R0: Skip Render
        uint32_t spos;                // R1: Sprite Position (0: Between BG0 and BG1 ~ 3)
        uint32_t scrollX[VDP_BG_NUM]; // R2-5: Scroll BGs X
        uint32_t scrollY[VDP_BG_NUM]; // R6-9: Scroll BGs Y
        uint32_t bmp[VDP_BG_NUM];     // R10-13: BGs Bitmap Mode
        uint32_t cls[VDP_BG_NUM + 1]; // R14-18: Clear Screen
        uint32_t g_bg;                // R19: Graphic Draw - BG number
        uint32_t g_x1;                // R20: Graphic Draw - X1
        uint32_t g_y1;                // R21: Graphic Draw - Y1
        uint32_t g_x2;                // R22: Graphic Draw - X2
        uint32_t g_y2;                // R23: Graphic Draw - Y2
        uint32_t g_col;               // R24: Graphic Draw - Color (RGB888)
        uint32_t g_opt;               // R25: Graphic Draw - Option
        uint32_t g_exe;               // R26: Graphic Draw - Execute
        uint32_t skip0;               // R27: Skip Rendering BG0
        uint32_t skip1;               // R28: Skip Rendering BG1
        uint32_t skip2;               // R29: Skip Rendering BG2
        uint32_t skip3;               // R30: Skip Rendering BG3
        uint32_t pf_init;             // R31: Profotinaol Font - Initialize
        uint32_t pf_ptn;              // R32: Profotinaol Font - Pattern
        uint32_t pf_dx;               // R33: Profotinaol Font - diff X
        uint32_t pf_dy;               // R34: Profotinaol Font - diff Y
        uint32_t pf_width;            // R35: Profotinaol Font - width
        uint32_t cp_fr;               // R36: Copy Character Pattern (From)
        uint32_t cp_to;               // R37: Copy Character Pattern (To)
        uint32_t reserved[218];       // Reserved
    } Register;

    typedef struct {
        uint32_t visible;     // Visible (0 or not 0)
        int32_t y;            // Position (Y)
        int32_t x;            // Position (X)
        uint32_t attr;        // Attribute
        uint32_t size;        // Size (0: 8x8, 1: 16x16, 2: 24x24, 3: 32x32 ... 31: 256x256)
        int32_t rotate;       // Rotate (-360 ~ 360)
        uint32_t scale;       // Scale (0 ~ 400 percent)
        uint32_t alpha;       // Alpha (0x000000 ~ 0xFFFFFF)
        uint32_t mask;        // Mask (0: disabled, or RGB888)
        uint32_t sly;         // Scale Lock (Y)
        uint32_t slx;         // Scale Lock (X)
        uint32_t pri;         // High Priority Flag
        uint32_t ram_ptr;     // Bitmap Sprite Buffer (RGB888)
        uint32_t reserved[3]; // Reserved
    } OAM;

    typedef struct {
        int32_t dx;
        int32_t dy;
        int32_t width;
        int32_t reserved;
    } PropotionalInfo;

    struct Context {
        uint32_t display[VDP_DISPLAY_PIXELS]; // Virtual Display (scaled 2x in both axes)
        uint8_t ptn[65536][32];               // Character Pattern (ROM)
        uint32_t nametbl[VDP_BG_NUM][65536];  // Name Table
        OAM oam[1024];                        // OAM
        uint32_t palette[16][16];             // Palette
        PropotionalInfo pinfo[0x80];          // Propotional Info
        Register reg;                         // Register
        int wx1[VDP_BG_NUM];                  // BG Window X1
        int wy1[VDP_BG_NUM];                  // BG Window Y1
        int wx2[VDP_BG_NUM];                  // BG Window X2
        int wy2[VDP_BG_NUM];                  // BG Window Y2
    } ctx;

    VDP()
    {
        this->rom.ptn.clear();
        this->rom.pal = nullptr;
        this->rom.palSize = 0;
    }

    void setCpuRam(const uint8_t* cpu_ram)
    {
        this->cpu_ram = cpu_ram;
    }

    ~VDP()
    {
        for (auto ptn : this->rom.ptn) {
            delete ptn;
        }
        this->rom.ptn.clear();
    }

    void reset()
    {
        memset(this->ctx.display, 0, sizeof(this->ctx.display));
        memset(this->ctx.nametbl, 0, sizeof(this->ctx.nametbl));
        memset(this->ctx.oam, 0, sizeof(this->ctx.oam));
        memset(&this->ctx.reg, 0, sizeof(Register));
        for (int i = 0; i < VDP_BG_NUM; i++) {
            this->ctx.wx1[i] = 0;
            this->ctx.wy1[i] = 0;
            this->ctx.wx2[i] = VDP_WIDTH - 1;
            this->ctx.wy2[i] = VDP_HEIGHT - 1;
        }
        this->resetPattern();
        this->resetPalette();
    }

    void addPattern(int index, const void* ptn, size_t ptnSize)
    {
        this->rom.ptn.push_back(new PatternRom(index, (const uint8_t*)ptn, (int)ptnSize));
    }

    void setPalette(const void* pal, size_t palSize)
    {
        this->rom.pal = (const uint8_t*)pal;
        this->rom.palSize = palSize;
    }

    uint32_t read(uint32_t address)
    {
        if (0xC00000 <= address && address < 0xD00000) {
            uint8_t n = (address & 0xC0000) >> 18;
            uint16_t addr = (address & 0x3FFFC) >> 2;
            return this->ctx.nametbl[n][addr];
        } else {
            switch (address & 0xFF0000) {
                case 0xD00000: {
                    uint16_t index = (address & 0xFFC0) >> 6;
                    uint8_t arg = (address & 0x003C) >> 2;
                    uint32_t* rawOam = (uint32_t*)&this->ctx.oam[index];
                    return rawOam[arg];
                }
                case 0xD10000: {
                    uint8_t pn = (address & 0x3C0) >> 6;
                    uint8_t cn = (address & 0x03C) >> 2;
                    return this->ctx.palette[pn][cn];
                }
                case 0xD20000: {
                    uint8_t index = (address & 0x3FC) >> 2;
                    uint32_t* rawReg = (uint32_t*)&this->ctx.reg;
                    switch (index) {
                        case 26: return this->readPixel();
                        case 33: return this->ctx.pinfo[this->ctx.reg.pf_ptn & 0x7F].dx;
                        case 34: return this->ctx.pinfo[this->ctx.reg.pf_ptn & 0x7F].dy;
                        case 35: return this->ctx.pinfo[this->ctx.reg.pf_ptn & 0x7F].width;
                    }
                    return rawReg[index];
                }
            }
        }
        return 0xFFFFFFFF;
    }

    void write(uint32_t address, uint32_t value)
    {
        if (0xC00000 <= address && address < 0xD00000) {
            uint8_t n = (address & 0xC0000) >> 18;
            uint16_t addr = (address & 0x3FFFC) >> 2;
            this->ctx.nametbl[n][addr] = value;
        } else {
            switch (address & 0xFF0000) {
                case 0xD00000: {
                    uint16_t index = (address & 0xFFC0) >> 6;
                    uint8_t arg = (address & 0x003C) >> 2;
                    uint32_t* rawOam = (uint32_t*)&this->ctx.oam[index];
                    rawOam[arg] = value;
                    return;
                }
                case 0xD10000: {
                    uint8_t pn = (address & 0x3C0) >> 6;
                    uint8_t cn = (address & 0x03C) >> 2;
                    this->ctx.palette[pn][cn] = value;
                    return;
                }
                case 0xD20000: {
                    uint8_t index = (address & 0x3FC) >> 2;
                    uint32_t* rawReg = (uint32_t*)&this->ctx.reg;
                    rawReg[index] = value;
                    switch (index) {
                        case 2: this->bitmapScrollX(0, (int)value); break;
                        case 3: this->bitmapScrollX(1, (int)value); break;
                        case 4: this->bitmapScrollX(2, (int)value); break;
                        case 5: this->bitmapScrollX(3, (int)value); break;
                        case 6: this->bitmapScrollY(0, (int)value); break;
                        case 7: this->bitmapScrollY(1, (int)value); break;
                        case 8: this->bitmapScrollY(2, (int)value); break;
                        case 9: this->bitmapScrollY(3, (int)value); break;
                        case 14: this->cls(value); break;
                        case 15: this->cls(0, value); break;
                        case 16: this->cls(1, value); break;
                        case 17: this->cls(2, value); break;
                        case 18: this->cls(3, value); break;
                        case 26: this->graphicDraw(value); break;
                        case 31: this->setupPropotional(value & 0xFFFF); break;
                        case 33: this->ctx.pinfo[this->ctx.reg.pf_ptn & 0x7F].dx = value; break;
                        case 34: this->ctx.pinfo[this->ctx.reg.pf_ptn & 0x7F].dy = value; break;
                        case 35: this->ctx.pinfo[this->ctx.reg.pf_ptn & 0x7F].width = value; break;
                        case 37: this->copyCharacterPattern(); break;
                    }
                    return;
                }
            }
        }
    }

    void render()
    {
        if (this->ctx.reg.skip) {
            return;
        }
        for (int i = 0; i < VDP_DISPLAY_PIXELS; i++) {
            this->ctx.display[i] = this->ctx.palette[0][0];
        }
        for (int i = 0; i < VDP_BG_NUM; i++) {
            if (0 == ((uint32_t*)&this->ctx.reg.skip0)[i]) {
                this->renderBG(i);
            }
            if (i == this->ctx.reg.spos) {
                this->renderSprites();
            }
        }
    }

  private:
    void copyCharacterPattern()
    {
        this->ctx.reg.cp_fr &= 0xFFFF;
        this->ctx.reg.cp_to &= 0xFFFF;
        if (this->ctx.reg.cp_fr == this->ctx.reg.cp_to) {
            return;
        }
        memcpy(this->ctx.ptn[this->ctx.reg.cp_to], this->ctx.ptn[this->ctx.reg.cp_fr], 32);
    }

    void setupPropotional(uint16_t ptn_start)
    {
        if (0x10000 - 0x80 < ptn_start) {
            return;
        }
        uint8_t* ptn = this->ctx.ptn[ptn_start];
        for (int i = 0; i < 0x80; i++) {
            int sx = 8;
            int ex = -1;
            for (int y = 0; y < 8; y++) {
                int px[8];
                px[0] = (ptn[0] & 0xF0) >> 4;
                px[1] = ptn[0] & 0x0F;
                px[2] = (ptn[1] & 0xF0) >> 4;
                px[3] = ptn[1] & 0x0F;
                px[4] = (ptn[2] & 0xF0) >> 4;
                px[5] = ptn[2] & 0x0F;
                px[6] = (ptn[3] & 0xF0) >> 4;
                px[7] = ptn[3] & 0x0F;
                ptn += 4;
                for (int x = 0; x < 8; x++) {
                    if (px[x]) {
                        if (x < sx) {
                            sx = x;
                        }
                    }
                    if (px[7 - x]) {
                        if (ex < 7 - x) {
                            ex = 7 - x;
                        }
                    }
                }
            }
            if (0 < ex) {
                this->ctx.pinfo[i].dx = -sx;
                this->ctx.pinfo[i].width = (ex - sx) + 2;
                switch (i) {
                    case '_':
                    case '.':
                    case ',':
                    case 'g':
                    case 'j':
                        this->ctx.pinfo[i].dy = 1;
                        break;
                    case 'p':
                    case 'q':
                    case 'y':
                        this->ctx.pinfo[i].dy = 2;
                        break;
                    default:
                        this->ctx.pinfo[i].dy = 0;
                }
            } else {
                this->ctx.pinfo[i].dx = 0;
                this->ctx.pinfo[i].dy = 0;
                if (i == ' ' || i == '\t') {
                    this->ctx.pinfo[i].width = 4; // space width
                } else {
                    this->ctx.pinfo[i].width = 0;
                }
            }
            // printf("%c: dx=%d, dy=%d, width=%d\n", isprint(i) ? i : '?', this->ctx.pinfo[i].dx, this->ctx.pinfo[i].dy, this->ctx.pinfo[i].width);
        }
    }

    inline void cls(uint32_t value)
    {
        for (int i = 0; i < 4; i++) {
            this->cls(i, value);
        }
    }

    inline void cls(int n, uint32_t value)
    {
        n &= 3;
        uint8_t* cp = (uint8_t*)&value;
        if (cp[0] == cp[1] && cp[1] == cp[2] && cp[2] == cp[3]) {
            memset(this->ctx.nametbl[n], *cp, 0x40000);
        } else {
            for (int i = 0; i < 0x10000; i++) {
                this->ctx.nametbl[n][i] = value;
            }
        }
    }

    inline void graphicDraw(uint32_t op)
    {
        static void (*func[])(VDP*) = {
            graphicDrawPixel,
            graphicDrawLine,
            graphicDrawBox,
            graphicDrawBoxFill,
            graphicDrawCharacter,
            graphicDrawJisX0201,
            graphicDrawJisX0208,
            graphicDrawClear,
            graphicDrawWindow,
        };
        if (op < 9) {
            func[op](this);
        }
    }

    inline uint32_t readPixel()
    {
        int bg = this->ctx.reg.g_bg & 3;
        int x1 = (int)this->ctx.reg.g_x1;
        int y1 = (int)this->ctx.reg.g_y1;
        if (x1 < 0 || VDP_WIDTH <= x1 || y1 < 0 || VDP_HEIGHT <= y1) {
            return 0;
        } else {
            return this->ctx.nametbl[bg][y1 * VDP_WIDTH + x1];
        }
    }

    inline void bitmapScrollX(int bg, int vector)
    {
        if (!this->ctx.reg.bmp[bg]) {
            return; // Not Bitmap Mode
        }
        if (0 == vector) {
            return;
        }
        if (VDP_WIDTH <= vector || vector <= -VDP_WIDTH) {
            this->cls(bg, 0);
            return;
        }
        uint32_t* vram = this->ctx.nametbl[bg];
        if (vector < 0) {
            // left scroll
            vector = -vector;
            for (int i = 0; i < VDP_HEIGHT; i++) {
                memmove(vram, &vram[vector], (VDP_WIDTH - vector) * 4);
                memset(&vram[VDP_WIDTH - vector], 0, vector * 4);
                vram += VDP_WIDTH;
            }
        } else {
            // right scroll
            for (int i = 0; i < VDP_HEIGHT; i++) {
                memmove(&vram[vector], vram, (VDP_WIDTH - vector) * 4);
                memset(vram, 0, vector * 4);
                vram += VDP_WIDTH;
            }
        }
    }

    inline void bitmapScrollY(int bg, int vector)
    {
        if (!this->ctx.reg.bmp[bg] || 0 == vector) {
            return;
        }
        if (VDP_HEIGHT <= vector || vector <= -VDP_HEIGHT) {
            this->cls(bg, 0);
            return;
        }
        uint32_t* vram = this->ctx.nametbl[bg];
        if (vector < 0) {
            // upward scroll
            vector = -vector;
            memmove(vram, &vram[vector * VDP_WIDTH], (VDP_HEIGHT - vector) * VDP_WIDTH * 4);
            memset(&vram[(VDP_HEIGHT - vector) * VDP_WIDTH], 0, vector * VDP_WIDTH * 4);
        } else {
            // down scroll
            memmove(&vram[vector * VDP_WIDTH], vram, (VDP_HEIGHT - vector) * VDP_WIDTH * 4);
            memset(vram, 0, vector * VDP_WIDTH * 4);
        }
    }

    inline uint8_t readPatternPixel(uint16_t tile, int px, int py)
    {
        const uint8_t* data = this->ctx.ptn[tile];
        data += (py & 7) << 2;
        data += (px & 6) >> 1;
        const uint8_t raw = *data;
        return (px & 1) ? (raw & 0x0F) : (raw >> 4);
    }

    inline uint8_t readSpritePixel(uint16_t base, int psize, int px, int py)
    {
        const int tileOffset = ((py >> 3) * psize) + (px >> 3);
        return readPatternPixel((base + tileOffset) & 0xFFFF, px & 7, py & 7);
    }

    inline void renderBG(int n)
    {
        uint32_t* display = this->ctx.display;
        const int scaledWidth = VDP_DISPLAY_WIDTH;
        if (this->ctx.reg.bmp[n]) {
            // Bitmap Mode
            uint32_t* vram = this->ctx.nametbl[n];
            for (int y = this->ctx.wy1[n]; y <= this->ctx.wy2[n]; y++) {
                int srcPtr = y * VDP_WIDTH + this->ctx.wx1[n];
                int dstTop = (y * VDP_DISPLAY_SCALE) * scaledWidth + this->ctx.wx1[n] * VDP_DISPLAY_SCALE;
                int dstBottom = dstTop + scaledWidth;
                for (int x = this->ctx.wx1[n]; x <= this->ctx.wx2[n]; x++, srcPtr++) {
                    uint32_t col = vram[srcPtr];
                    if (col) {
                        display[dstTop] = col;
                        display[dstTop + 1] = col;
                        display[dstBottom] = col;
                        display[dstBottom + 1] = col;
                    }
                    dstTop += VDP_DISPLAY_SCALE;
                    dstBottom += VDP_DISPLAY_SCALE;
                }
            }
        } else {
            // Character Pattern Mode
            for (int dy = 0; dy < VDP_HEIGHT; dy++) {
                auto wy = (dy + this->ctx.reg.scrollY[n]) & 0x7FF;
                int dstTop = (dy * VDP_DISPLAY_SCALE) * scaledWidth;
                int dstBottom = dstTop + scaledWidth;
                for (int dx = 0; dx < VDP_WIDTH; dx++) {
                    auto wx = (dx + this->ctx.reg.scrollX[n]) & 0x7FF;
                    auto attr = this->ctx.nametbl[n][(((wy >> 3) & 0xFF) << 8) | (wx >> 3) & 0xFF];
                    const bool flipH = (attr & 0x80000000) != 0;
                    const bool flipV = (attr & 0x40000000) != 0;
                    uint8_t pal = (attr & 0xF0000) >> 16;
                    int px = wx & 7;
                    int py = wy & 7;
                    if (flipH) px = 7 - px;
                    if (flipV) py = 7 - py;
                    const uint8_t col = readPatternPixel(attr & 0xFFFF, px, py);
                    if (col) {
                        uint32_t color = this->ctx.palette[pal][col];
                        display[dstTop] = color;
                        display[dstTop + 1] = color;
                        display[dstBottom] = color;
                        display[dstBottom + 1] = color;
                    }
                    dstTop += VDP_DISPLAY_SCALE;
                    dstBottom += VDP_DISPLAY_SCALE;
                }
            }
        }
    }

    inline void renderSprites()
    {
        for (int i = 1023; 0 <= i; i--) {
            if (this->ctx.oam[i].visible && !this->ctx.oam[i].pri) {
                renderSprite(&this->ctx.oam[i]);
            }
        }
        for (int i = 1023; 0 <= i; i--) {
            if (this->ctx.oam[i].visible && this->ctx.oam[i].pri) {
                renderSprite(&this->ctx.oam[i]);
            }
        }
    }

    inline void renderSprite(OAM* oam)
    {
        int psize = (oam->size & 0x1F) + 1; // 1 ~ 32
        int size = psize << 3;              // 8 ~ 256
        int ptn = oam->attr & 0xFFFF;
        int pal = (oam->attr & 0xF0000) >> 16;
        bool flipH = (oam->attr & 0x80000000) ? true : false;
        bool flipV = (oam->attr & 0x40000000) ? true : false;
        int32_t angle = 90 - oam->rotate;
        const int coordScale = VDP_DISPLAY_SCALE;
        const int displayWidth = VDP_DISPLAY_WIDTH;
        const int displayHeight = VDP_DISPLAY_HEIGHT;
        int scale = oam->scale;
        if (scale == 0 || oam->alpha == 0) {
            return;
        }
        scale *= coordScale;
        if (scale > 800) {
            scale = 800;
        }
        angle %= 360;
        if (angle < 0) {
            angle += 360;
        }
        // Scale & Rotate
        int scaledSizeX = oam->slx ? size * 200 / 100 : size * scale / 100;
        int scaledSizeY = oam->sly ? size * 200 / 100 : size * scale / 100;
        double ratioX = size;
        ratioX /= scaledSizeX;
        double ratioY = size;
        ratioY /= scaledSizeY;
        int offsetX = ((size * 2 - scaledSizeX) / 2);
        int offsetY = ((size * 2 - scaledSizeY) / 2);
        int halfSizeX = scaledSizeX / 2;
        int halfSizeY = scaledSizeY / 2;
        int scaledX = oam->x * coordScale;
        int scaledY = oam->y * coordScale;
        for (int dy = scaledY + offsetY, by = 0; by < scaledSizeY; dy++, by++) {
            int py = (int)(by * ratioY);
            int wy = flipH ? size - py - 1 : py;
            for (int dx = scaledX + offsetX, bx = 0; bx < scaledSizeX; dx++, bx++) {
                int px = (int)(bx * ratioX);
                int wx = flipH ? size - px - 1 : px;
                int ddy = ((by - halfSizeY) * vgsx_sin[angle] + (bx - halfSizeX) * vgsx_cos[angle]) / 256 + halfSizeY;
                ddy += scaledY + offsetY;
                if (ddy < 0 || displayHeight <= ddy) {
                    continue; // Out of screen top (check next line)
                }
                int ddx = ((bx - halfSizeX) * vgsx_sin[angle] - (by - halfSizeY) * vgsx_cos[angle]) / 256 + halfSizeX;
                ddx += scaledX + offsetX;
                if (ddx < 0 || displayWidth <= ddx) {
                    continue; // Out of screen left (check next pixel)
                }
                // Render Pixel
                if (oam->ram_ptr) {
                    const int ram_ptr = (oam->ram_ptr + (wx + (wy * (1 + psize) * 8)) * 4) & 0xFFFFC;
                    if (angle % 90 && ddx + 1 < displayWidth) {
                        uint32_t rgb = cpu_ram[ram_ptr + 1];
                        rgb <<= 8;
                        rgb |= cpu_ram[ram_ptr + 2];
                        rgb <<= 8;
                        rgb |= cpu_ram[ram_ptr + 3];
                        this->renderSpritePixel(ddy * displayWidth + ddx + 1, rgb, oam->alpha, oam->mask);
                    }
                } else {
                    const uint8_t col = readSpritePixel(ptn, psize, wx, wy);
                    if (col) {
                        this->renderSpritePixel(ddy * displayWidth + ddx, this->ctx.palette[pal][col], oam->alpha, oam->mask);
                        if (angle % 90 && ddx + 1 < displayWidth) {
                            this->renderSpritePixel(ddy * displayWidth + ddx + 1, this->ctx.palette[pal][col], oam->alpha, oam->mask);
                        }
                    }
                }
            }
        }
    }

    inline void renderSpritePixel(int displayAddress, uint32_t color, uint32_t alpha, uint32_t mask)
    {
        if (mask) {
            color = mask;
        }
        if (0 == (alpha & 0xFFFFFF)) {
            return;
        }
        if (0xFFFFFF == (alpha & 0xFFFFFF)) {
            this->ctx.display[displayAddress] = color;
            return;
        }
        uint32_t src = this->ctx.display[displayAddress];
        uint32_t ar = (alpha & 0xFF0000) >> 16;
        uint32_t ag = (alpha & 0x00FF00) >> 8;
        uint32_t ab = alpha & 0x0000FF;
        uint32_t sr = (src & 0xFF0000) >> 16;
        uint32_t sg = (src & 0x00FF00) >> 8;
        uint32_t sb = src & 0x0000FF;
        uint32_t dr = (color & 0xFF0000) >> 16;
        uint32_t dg = (color & 0x00FF00) >> 8;
        uint32_t db = color & 0x0000FF;
        sr *= (255 - ar);
        sg *= (255 - ag);
        sb *= (255 - ab);
        dr *= ar;
        dg *= ag;
        db *= ab;
        uint32_t color2 = ((sr + dr) & 0x00FF00) << 8;
        color2 |= (sg + dg) & 0x00FF00;
        color2 |= ((sb + db) & 0x00FF00) >> 8;
        this->ctx.display[displayAddress] = color2;
    }
};

static inline int _abs(int value) { return value < 0 ? -value : value; }
static inline int _sgn(int value) { return value < 0 ? -1 : 1; }

static inline void drawPixel(uint32_t* vram, int32_t x1, int32_t y1, uint32_t col)
{
    if (x1 < 0 || VDP_WIDTH <= x1 || y1 < 0 || VDP_HEIGHT <= y1) {
        return;
    }
    vram[y1 * VDP_WIDTH + x1] = col;
}

static inline void drawLine(uint32_t* vram, int32_t fx, int32_t fy, int32_t tx, int32_t ty, uint32_t col)
{
    int idx, idy;
    int ia, ib, ie;
    int w;
    idx = tx - fx;
    idy = ty - fy;
    if (!idx || !idy) {
        if (tx < fx) {
            w = fx;
            fx = tx;
            tx = w;
        }
        if (ty < fy) {
            w = fy;
            fy = ty;
            ty = w;
        }
        if (0 == idy) {
            for (; fx <= tx; fx++) {
                drawPixel(vram, fx, fy, col);
            }
        } else {
            for (; fy <= ty; fy++) {
                drawPixel(vram, fx, fy, col);
            }
        }
        return;
    }
    w = 1;
    ia = _abs(idx);
    ib = _abs(idy);
    if (ia >= ib) {
        ie = -_abs(idy);
        while (w) {
            drawPixel(vram, fx, fy, col);
            if (fx == tx) break;
            fx += _sgn(idx);
            ie += 2 * ib;
            if (ie >= 0) {
                fy += _sgn(idy);
                ie -= 2 * ia;
            }
        }
    } else {
        ie = -_abs(idx);
        while (w) {
            drawPixel(vram, fx, fy, col);
            if (fy == ty) break;
            fy += _sgn(idy);
            ie += 2 * ia;
            if (ie >= 0) {
                fx += _sgn(idx);
                ie -= 2 * ib;
            }
        }
    }
}

static inline void graphicDrawPixel(VDP* vdp)
{
    drawPixel(vdp->ctx.nametbl[vdp->ctx.reg.g_bg & 3],
              (int32_t)vdp->ctx.reg.g_x1,
              (int32_t)vdp->ctx.reg.g_y1,
              vdp->ctx.reg.g_col);
}

static inline void graphicDrawLine(VDP* vdp)
{
    drawLine(vdp->ctx.nametbl[vdp->ctx.reg.g_bg & 3],
             (int32_t)vdp->ctx.reg.g_x1,
             (int32_t)vdp->ctx.reg.g_y1,
             (int32_t)vdp->ctx.reg.g_x2,
             (int32_t)vdp->ctx.reg.g_y2,
             vdp->ctx.reg.g_col);
}

static inline void graphicDrawBox(VDP* vdp)
{
    uint32_t* vram = vdp->ctx.nametbl[vdp->ctx.reg.g_bg & 3];
    int32_t fx = (int32_t)vdp->ctx.reg.g_x1;
    int32_t fy = (int32_t)vdp->ctx.reg.g_y1;
    int32_t tx = (int32_t)vdp->ctx.reg.g_x2;
    int32_t ty = (int32_t)vdp->ctx.reg.g_y2;
    uint32_t col = vdp->ctx.reg.g_col;
    drawLine(vram, fx, fy, tx, fy, col);
    drawLine(vram, fx, fy, fx, ty, col);
    drawLine(vram, tx, ty, tx, fy, col);
    drawLine(vram, tx, ty, fx, ty, col);
}

static inline void graphicDrawBoxFill(VDP* vdp)
{
    uint32_t* vram = vdp->ctx.nametbl[vdp->ctx.reg.g_bg & 3];
    int32_t fx = (int32_t)vdp->ctx.reg.g_x1;
    int32_t fy = (int32_t)vdp->ctx.reg.g_y1;
    int32_t tx = (int32_t)vdp->ctx.reg.g_x2;
    int32_t ty = (int32_t)vdp->ctx.reg.g_y2;
    uint32_t col = vdp->ctx.reg.g_col;
    if (ty < fy) {
        int w = fy;
        fy = ty;
        ty = w;
    }
    for (int32_t y = fy; y < ty; y++) {
        drawLine(vram, tx, y, fx, y, col);
    }
}

static inline void graphicDrawCharacter(VDP* vdp)
{
    uint32_t* vram = vdp->ctx.nametbl[vdp->ctx.reg.g_bg & 3];
    int32_t x = (int32_t)vdp->ctx.reg.g_x1;
    int32_t y = (int32_t)vdp->ctx.reg.g_y1;
    uint8_t pal = (int32_t)vdp->ctx.reg.g_col & 0x0F;
    uint8_t* ptn = vdp->ctx.ptn[vdp->ctx.reg.g_opt & 0xFFFF];
    bool drawZero = vdp->ctx.reg.g_col & 0x80000000 ? true : false;

    if (x < -8 || y < -8 || 320 <= x || 200 <= y) {
        return;
    }
    for (int i = 0; i < 8; i++) {
        if (y + i < 0 || 200 <= y + i) {
            continue;
        }
        for (int j = 0; j < 4; j++) {
            int p0 = (ptn[i * 4 + j] & 0xF0) >> 4;
            int p1 = ptn[i * 4 + j] & 0x0F;
            uint32_t c0 = vdp->ctx.palette[pal][p0];
            uint32_t c1 = vdp->ctx.palette[pal][p1];
            if (0 <= x + j * 2 && x + j * 2 < 320) {
                if (p0) {
                    vram[(y + i) * 320 + x + j * 2] = c0;
                } else if (drawZero) {
                    vram[(y + i) * 320 + x + j * 2] = 0;
                }
            }
            if (0 <= x + j * 2 + 1 && x + j * 2 + 1 < 320) {
                if (p1) {
                    vram[(y + i) * 320 + x + j * 2 + 1] = c1;
                } else if (drawZero) {
                    vram[(y + i) * 320 + x + j * 2 + 1] = 0;
                }
            }
        }
    }
}

static inline void graphicDrawJisX0201(VDP* vdp)
{
    uint32_t* vram = vdp->ctx.nametbl[vdp->ctx.reg.g_bg & 3];
    int32_t x = (int32_t)vdp->ctx.reg.g_x1;
    int32_t y = (int32_t)vdp->ctx.reg.g_y1;
    uint32_t col = vdp->ctx.reg.g_col;
    uint32_t code = (int32_t)vdp->ctx.reg.g_opt;
    if (255 < code) {
        return;
    }
    const uint8_t* ptn = &k8x12_jisx0201[code * 12];
    for (int iy = 0; iy < 12; iy++) {
        int p = ptn[iy];
        if (y + iy < 0 || 200 <= y + iy) {
            continue;
        }
        for (int ix = 0; ix < 4; ix++) {
            if (x + ix < 0 || 320 <= x + ix) {
                continue;
            }
            if (p & 0x80) {
                vram[(y + iy) * 320 + x + ix] = col;
            }
            p <<= 1;
        }
    }
}

static inline void graphicDrawJisX0208(VDP* vdp)
{
    uint32_t* vram = vdp->ctx.nametbl[vdp->ctx.reg.g_bg & 3];
    int32_t x = (int32_t)vdp->ctx.reg.g_x1;
    int32_t y = (int32_t)vdp->ctx.reg.g_y1;
    uint32_t col = vdp->ctx.reg.g_col;
    uint32_t code = (int32_t)vdp->ctx.reg.g_opt;
    printf("jisx0208: 0x%04X\n", code);
    if (8835 < code) {
        return;
    }
    const uint8_t* ptn = &k8x12_jisx0208[code * 12];
    for (int iy = 0; iy < 12; iy++) {
        int p = ptn[iy];
        if (y + iy < 0 || 200 <= y + iy) {
            continue;
        }
        for (int ix = 0; ix < 8; ix++) {
            if (x + ix < 0 || 320 <= x + ix) {
                continue;
            }
            if (p & 0x80) {
                vram[(y + iy) * 320 + x + ix] = col;
            }
            p <<= 1;
        }
    }
}

static inline void graphicDrawClear(VDP* vdp)
{
    int x1 = range((int)vdp->ctx.reg.g_x1, 0, 319);
    int y1 = range((int)vdp->ctx.reg.g_y1, 0, 199);
    int x2 = range((int)vdp->ctx.reg.g_x2, 0, 319);
    int y2 = range((int)vdp->ctx.reg.g_y2, 0, 199);
    if (x2 < x1) {
        int w = x1;
        x1 = x2;
        x2 = w;
    }
    if (y2 < y1) {
        int w = y1;
        y1 = y2;
        y2 = w;
    }
    uint32_t* vram = &vdp->ctx.nametbl[vdp->ctx.reg.g_bg & 3][y1 * VDP_WIDTH];
    for (int i = 0; i <= y2 - y1; i++, vram += VDP_WIDTH) {
        memset(&vram[x1], 0, (x2 - x1 + 1) * 4);
    }
}

static inline void graphicDrawWindow(VDP* vdp)
{
    int n = vdp->ctx.reg.g_bg & 3;
    int x1 = range((int)vdp->ctx.reg.g_x1, 0, 319);
    int y1 = range((int)vdp->ctx.reg.g_y1, 0, 199);
    int x2 = range((int)vdp->ctx.reg.g_x2, 0, 319);
    int y2 = range((int)vdp->ctx.reg.g_y2, 0, 199);
    if (x2 < x1) {
        int w = x1;
        x1 = x2;
        x2 = w;
    }
    if (y2 < y1) {
        int w = y1;
        y1 = y2;
        y2 = w;
    }
    vdp->ctx.wx1[n] = x1;
    vdp->ctx.wy1[n] = y1;
    vdp->ctx.wx2[n] = x2;
    vdp->ctx.wy2[n] = y2;
}
