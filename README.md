# VGS-X

## WIP

This project is still in the WIP; _Work In Progress_ phase.

Please note that the current specifications are subject to frequent, casual breaking changes.

Once this project reaches a stable phase, version 1.0.0 is scheduled for release.

Status

1. [x] Execute MC680x0 ELF32 module
2. [x] Implement VDP
3. [x] Implement Background Music function
4. [x] Implement Sound Effect function
5. [x] Implement Gamepad function
6. [x] Release beta 0.1.0
7. [ ] Make launch title for VGS-X

Changes after Version 0.1.0 can be found in [CHANGES.md](./CHANGES.md).

## About VGS-X

The VGS-X is a 16-bit game console featuring an MC68030 processor, the FM sound chips, and a proprietary VDP optimized for MC68k architecture.

Games can be developed using the GCC; _GNU Compiler Collection_ for MC68k.

Supported development environment operating systems are **Ubuntu Linux** or **macOS**. _(If you want to use Windows as a development machine, please use WSL2.)_

The runtime environment supports the all of PC operating systems (Windows, macOS, and Linux) that supported by Steam Client.

In the future, we also plan to provide runtimes capable of running on Nintendo Switch 1/2 and PlayStation 4/5. Due to an NDA, we cannot disclose details, but we will be confirmed that the [core](./src/core) modules can be built and run using the SDKs for those game consoles.

VGS-X aims to provide game developers and publishers with an environment that enables them to deliver games that are fully compatible across any computer with certain performance specifications.

## Setup Build Environment

Since VGS-X can execute MC68030 ELF format modules, you must install `m68k-elf-gcc` to develop games for VGS-X.

On macOS, it can be easily installed via Homebrew. However, since no apt package is provided for Ubuntu Linux, you will need to build it yourself.

### Setup Build Environment: macOS

Please install `m68k-elf-gcc` and `SDL2` in an environment where Xcode and Homebrew are installed.

```bash
brew install m68k-elf-gcc
brew install sdl2
```

### Setup Build Environment: Ubuntu Linux

Below are the installation steps required for game development for the VGS-X.

```bash
# Install Dependencies
sudo apt update
sudo apt install build-essential bison flex libgmp-dev libmpc-dev libmpfr-dev texinfo libncurses5-dev

# Install SDL2
sudo apt install build-essential libsdl2-dev libasound2 libasound2-dev

# Make a work directory for build m68k-elf
mkdir ~/m68k-work

# Build and Install the binutils for MC68k
cd ~/m68k-work
wget https://ftp.gnu.org/gnu/binutils/binutils-2.40.tar.gz
tar xvf binutils-2.40.tar.gz
cd binutils-2.40
mkdir ../binutils-build
cd ../binutils-build
../binutils-2.40/configure --target=m68k-elf --prefix=/usr/local/m68k-elf --disable-nls --disable-werror
make -j$(nproc)
sudo make install
export PATH=$PATH:/usr/local/m68k-elf/bin

# Build and Install GCC for MC68k
cd ~/m68k-work
wget https://ftp.gnu.org/gnu/gcc/gcc-12.2.0/gcc-12.2.0.tar.gz
tar xvf gcc-12.2.0.tar.gz
cd gcc-12.2.0
./contrib/download_prerequisites
mkdir ../gcc-build
cd ../gcc-build
../gcc-12.2.0/configure --target=m68k-elf --prefix=/usr/local/m68k-elf --enable-languages=c --disable-nls --disable-libssp --without-headers
make all-gcc -j$(nproc)
make all-target-libgcc -j$(nproc)
sudo make install-gcc install-target-libgcc
```

Add the following line to ~/.zprofile so that the path is set when Terminal launches.

```.zprofile
export PATH=$PATH:/usr/local/m68k-elf/bin
```

> We referenced the following article.
>
> [https://computeralgebra.hatenablog.com/entry/2025/02/26/233231](https://computeralgebra.hatenablog.com/entry/2025/02/26/233231).
>
> We would like to thank the author for writing it!

## Build and Execute an Example

If you've finished installing `m68k-elf-gcc`, you're now ready to start developing games for the VGS-X.

The following steps show how to obtain this repository using `git clone` and then run the example that displays “HELLO, WORLD!”.

```bash
git clone https://github.com/suzukiplan/vgsx
cd vgsx/example/01_hello
make
```

![screen shot](./example/01_hello/screen.png)

# Architecture Reference Manual

The following sections provide technical information useful for programming with VGS-X.

## Screen Specification

- It has a fixed screen resolution of **320x200** pixels.
- It has 4 layers of BGs and 1 layer of sprites.
- Each BGs has two modes: [Character Pattern Mode](#character-pattern) and [Bitmap Mode](#0xd20028-0xd20034-bitmap-mode).
- Sprites can display up to 1024.

> _The screen resolution of VGS-X (320x200) is designed to enable full-screen display on the SteamDeck (1280x800)._

## Memory Map

In VGS-X, the first 12MB (0x000000 ~ 0xBFFFFF) of the MC68030's 24-bit (16MB) address space is allocated for programs.

The final 1MB (0xF00000 ~ 0xFFFFFF) constitutes the WRAM; _Work RAM area_.

The space between the program and WRAM (0xC00000 ~ 0xEFFFFF = 3MB) constitutes the memory map for the VDP and [I/O](#io-map).

| Address             | Size    | Description  |
|:-------------------:|--------:|:-------------|
| 0x000000 ~ 0xBFFFFF | 12288KB | [Program (ELF module)](#program) |
| 0xC00000 ~ 0xCFFFFF |  1024KB | [Name Table](#name-table) |
| 0xD00000 ~ 0xD0FFFF |    64KB | [OAM](#oam-object-attribute-memory) |
| 0xD10000 ~ 0xD1FFFF |    64KB | [Palette](#palette) |
| 0xD20000 ~ 0xD2FFFF |    64KB | [VDP Register](#vdp-register) |
| 0xD30000 ~ 0xDFFFFF |   832KB | Reserved     |
| 0xE00000 ~ 0xEFFFFF |  1024KB | [I/O](#io-map) |
| 0xF00000 ~ 0xFFFFFF |  1024KB | WRAM         |

When accessing the address range 0xC00000 to 0xEFFFFF used as mmap, access must always be 32-bit aligned.

> Since the least significant two bits are always masked when accessing this address space, accesses to 0xC00000, 0xC00001, 0xC00002, and 0xC00003 always behave as if accessing 0xC00000.

[Character patterns](#character-pattern) and Sound data are stored in Read Only Memory, which cannot be directly referenced from the program. Like VGS-Zero, it is specified by pattern number.

## Program

The program area (0x000000 ~ 0xBFFFFF) contains the [ELF32 _(Executable and Linkable Format 32bit)_](https://refspecs.linuxfoundation.org/elf/gabi4+/ch4.intro.html) binary module.

When tune on the VGS-X, it begins executing the program from the entry point specified in the ELF header of the program loaded from the ROM cartridge.

Note that even if you wish to write your program solely in MC68k assembly language, you must always specify the ELF32 header and Program header that contains valid entry point and executable text.

> However, since there is no advantage to writing high-performance programs in assembly language on the VGS-X, we generally do not recommend writing programs entirely in assembly language.

The following command line the compilation options that must be specified when outputting programs that can run on VGS-X using m68k-elf-gcc.

```
m68k-elf-gcc
    -mc68030                     ... Compile as MC68030 binary
    -O2                          ... Optimize the Runtime Speed
    -I${VGSX_ROOT}/lib           ... Specify the VGS-X API header path using the -I option
    -o program                   
    program.c                    
    -L${VGSX_ROOT}/lib           ... Specify the VGS-X API archive path using the -L option
    -T${VGSX_ROOT}/lib/linker.ld ... Specify the linker.ld file describing the VGS-X memory map using the -T option.
    -Wl,-ecrt0                   ... Specify crt0 as the entry point when using the VGS-X runtime
```

Note that VGS-X does not provide the C standard library, but it does provide the [Runtime Library for VGS-X](#runtime-library-for-vgs-x).

> As an exception, you can use the `stdarg.h` provided by GCC.

## Character Pattern

VGS-X can use 65,536 character patterns.

One character pattern is 8x8 pixels.

The data consists of 32 bytes with the following bit layout:

| px0 | px1 | px2 | px3 | px4 | px5 | px6 | px7 | Line number |
| :-: | :-: | :-: | :-: | :-: | :-: | :-: | :-: | :---------- |
| H00 | L00 | H01 | L01 | H02 | L02 | H03 | L03 | Line 0      |
| H04 | L04 | H05 | L05 | H06 | L06 | H07 | L07 | Line 1      |
| H08 | L08 | H09 | L09 | H10 | L10 | H11 | L11 | Line 2      |
| H12 | L12 | H13 | L13 | H14 | L14 | H15 | L15 | Line 3      |
| H16 | L16 | H17 | L17 | H18 | L18 | H19 | L19 | Line 4      |
| H20 | L20 | H21 | L21 | H22 | L22 | H23 | L23 | Line 5      |
| H24 | L24 | H25 | L25 | H26 | L26 | H27 | L27 | Line 6      |
| H28 | L28 | H29 | L29 | H30 | L30 | H31 | L31 | Line 7      |

- `Hxx` : Upper 4 bits (0 ~ 15 = color number)
- `Lxx` : Lower 4 bits (0 ~ 15 = color number)

Remarks:

- This bit layout is compatible with VGS-Zero.
- Character patterns cannot be referenced directly from the program. You can draw the desired character by specifying the pattern number in the name table or OAM.
- Character pattern number is shared between BGs and Sprites.

> __WIP Note:__ Currently, the character pattern specification assumes that all necessary patterns are loaded at program startup. This means we intend to restrict dynamic pattern rewriting after startup. However, we also believe there is room to consider changing this specification.

## Palette

- VGS-X allows up to 16 palettes
- Each palette can contain 16 colors in RGB888 format
- Color number 0 is the transparent color
- Color number 0 in Palette number 0 becomes the backdrop (overall background) color.

| Address             | Palette Number | Color Number |
|:-------------------:|:--------------:|:------------:|
| 0xD10000 ~ 0xD10003 |        0       |        0     |
| 0xD10004 ~ 0xD10007 |        0       |        1     |
| 0xD10008 ~ 0xD1000B |        0       |        2     |
|          :          |        :       |        :     |
| 0xD10034 ~ 0xD10037 |        0       |       13     |
| 0xD10038 ~ 0xD1003B |        0       |       14     |
| 0xD1003C ~ 0xD1003F |        0       |       15     |
| 0xD10040 ~ 0xD10043 |        1       |        0     |
| 0xD10044 ~ 0xD10047 |        1       |        1     |
| 0xD10048 ~ 0xD1004B |        1       |        2     |
|          :          |        :       |        :     |
| 0xD103F4 ~ 0xD103F7 |       15       |       13     |
| 0xD103F8 ~ 0xD103FB |       15       |       14     |
| 0xD103FC ~ 0xD103FF |       15       |       15     |

Remarks:

- Bit Layout: `******** rrrrrrrr gggggggg bbbbbbbb`
- 0xD10400 ~ 0xD1FFFF is a mirror of 0xD10000 ~ 0xD103FF (1024 bytes).
- Please note that access to the palette table must always be 4-byte aligned.

## Name Table

- The Name Table is a 256x256 x 4bytes two-dimensional array of the [attributes](#attribute). 
- By setting character patterns and attribute data to it, graphics can be displayed on the background layer.
- The Name Table has a four-layer structure, with BG1 displayed on top of BG0, BG2 on top of BG1, BG3 on top of BG2, and BG4 on top of BG3.

| Address             | Size  | Name Table |
|:-------------------:|:-----:|:----------:|
| 0xC00000 ~ 0xC3FFFF | 256KB |     BG0    |
| 0xC40000 ~ 0xC7FFFF | 256KB |     BG1    |
| 0xC80000 ~ 0xCBFFFF | 256KB |     BG2    |
| 0xCC0000 ~ 0xCFFFFF | 256KB |     BG3    |

In [Bitmap Mode](#0xd20028-0xd20034-bitmap-mode), these areas corresponds to 320x200 pixels.

Please note that access to the name table must always be 4-byte aligned.

## Attribute

The Bit-Layout of the Name Table and OAM's attribute are as follows:

|  Bit  | Mnemonic | Description |
|:-----:|:--------:|:------------|
|   0   |   F/H    | Flip Horizontal |
|   1   |   F/V    | Flip Vertical |
|  2~7  | reserved | Specify 0 to maintain future compatibility. |
| 12~15 |   PAL    | [Palette](#palette) Number (0~15) |
| 16~31 |   PTN    | [Character Pattern](#character-pattern) Number (0~65535) |

## OAM (Object Attribute Memory)

OAM is a structure with the following attributes.

```c
typedef struct {
    uint32_t visible;      // Visible (0 or not 0)
    int32_t y;             // Position (Y)
    int32_t x;             // Position (X)
    uint32_t attr;         // Attribute
    uint32_t size;         // Size (0: 8x8, 1: 16x16, 2: 24x24, 3: 32x32 ... 31: 256x256)
    int32_t rotate;        // Rotate (-360 ~ 360)
    uint32_t scale;        // Scale (0: disabled or 1 ~ 400 percent)
    uint32_t reserved[9];  // Reserved
} OAM;
```

The specifications for each attribute are shown in the table below.

| Name    | Valid Range | Description |
|:--------|:-----------:|:------------|
| visible | 0 or 1      | Display sprite with a non-zero setting. |
| y       | -32768 ~ 32767 | Sprite display coordinates |
| x       | -32768 ~ 32767 | Sprite display coordinates |
| attr    | 32bit          | [Attribute](#attribute) |
| size    | 0 ~ 31         | [Size](#size-of-sprite) |
| rotate  | -360 ~ 360     | [Rotate](#rotate-of-sprite) |
| scale   | 0 ~ 400        | [Scale](#scale-of-sprite) |
| reserved| -              | Do not set a value other than zero. |

### (Size of Sprite)

Sprites are displayed as squares measuring `(size + 1) * 8` pixels.

For example, when size is set to 3 (32x32 pixels), the sprite is displayed using `16 = (size + 1) ^ 2` patterns. The layout of the pattern numbers at that time is as follows:

```
Size 3 Pattern Number Layout
+--------+--------+--------+--------+
|        |        |        |        |
| ptn+0  | ptn+1  | ptn+2  | ptn+3  |
|        |        |        |        |
+--------+--------+--------+--------+
|        |        |        |        |
| ptn+4  | ptn+5  | ptn+6  | ptn+7  |
|        |        |        |        |
+--------+--------+--------+--------+
|        |        |        |        |
| ptn+8  | ptn+9  | ptn+10 | ptn+11 |
|        |        |        |        |
+--------+--------+--------+--------+
|        |        |        |        |
| ptn+12 | ptn+13 | ptn+14 | ptn+15 |
|        |        |        |        |
+--------+--------+--------+--------+
```

### (Rotate of Sprite)

By specifying an angle (-360 to 360) for `rotate`, you can draw a rotated sprite.

Note that setting `rotate` to a non-zero value increases the sprite's drawing overhead.

### (Scale of Sprite)

You can specify the magnification rate as a percentage on the `scale`, either 0 (disabled) or within the range of 1 to 400.

## VDP Register

| Address | Name | Mnemonic | Description |
|:-------:|:----:|:--------:|:------------|
|0xD20000 |  R0  | SKIP     | [Skip Screen Update](#0xd20000-skip-screen-update) |
|0xD20004 |  R1  | SPOS     | [Sprites Position](#0xd20004-sprite-position) |
|0xD20008 |  R2  | SX0      | [Scroll X of BG0](#0xd20008-0xd20024-hardware-scroll) |
|0xD2000C |  R3  | SX1      | [Scroll X of BG1](#0xd20008-0xd20024-hardware-scroll) |
|0xD20010 |  R4  | SX2      | [Scroll X of BG2](#0xd20008-0xd20024-hardware-scroll) |
|0xD20014 |  R5  | SX3      | [Scroll X of BG3](#0xd20008-0xd20024-hardware-scroll) |
|0xD20018 |  R6  | SY0      | [Scroll Y of BG0](#0xd20008-0xd20024-hardware-scroll) |
|0xD2001C |  R7  | SY1      | [Scroll Y of BG1](#0xd20008-0xd20024-hardware-scroll) |
|0xD20020 |  R8  | SY2      | [Scroll Y of BG2](#0xd20008-0xd20024-hardware-scroll) |
|0xD20024 |  R9  | SY3      | [Scroll Y of BG3](#0xd20008-0xd20024-hardware-scroll) |
|0xD20028 |  R10 | BMP0     | [Bitmap Mode of BG0](#0xd20028-0xd20034-bitmap-mode) |
|0xD2002C |  R11 | BMP1     | [Bitmap Mode of BG1](#0xd20028-0xd20034-bitmap-mode) |
|0xD20030 |  R12 | BMP2     | [Bitmap Mode of BG2](#0xd20028-0xd20034-bitmap-mode) |
|0xD20034 |  R13 | BMP3     | [Bitmap Mode of BG3](#0xd20028-0xd20034-bitmap-mode) |
|0xD20038 |  R14 | CLSA     | [Clear Screen of All BGs](#0xd20038-0xd20048-clear-screen) |
|0xD2003C |  R15 | CLS0     | [Clear Screen of BG0](#0xd20038-0xd20048-clear-screen) |
|0xD20040 |  R16 | CLS1     | [Clear Screen of BG1](#0xd20038-0xd20048-clear-screen) |
|0xD20044 |  R17 | CLS2     | [Clear Screen of BG2](#0xd20038-0xd20048-clear-screen) |
|0xD20048 |  R18 | CLS3     | [Clear Screen of BG3](#0xd20038-0xd20048-clear-screen) |
|0xD2004C |  R19 | G_BG     | [Bitmap Graphic Draw](#0xd2004c-0xd20068-bitmap-graphic-draw) |
|0xD20050 |  R20 | G_X1     | [Bitmap Graphic Draw](#0xd2004c-0xd20068-bitmap-graphic-draw) |
|0xD20054 |  R21 | G_Y1     | [Bitmap Graphic Draw](#0xd2004c-0xd20068-bitmap-graphic-draw) |
|0xD20058 |  R22 | G_X2     | [Bitmap Graphic Draw](#0xd2004c-0xd20068-bitmap-graphic-draw) |
|0xD2005C |  R23 | G_Y2     | [Bitmap Graphic Draw](#0xd2004c-0xd20068-bitmap-graphic-draw) |
|0xD20060 |  R24 | G_COL    | [Bitmap Graphic Draw](#0xd2004c-0xd20068-bitmap-graphic-draw) |
|0xD20064 |  R25 | G_OPT    | [Bitmap Graphic Draw](#0xd2004c-0xd20068-bitmap-graphic-draw) |
|0xD20068 |  R26 | G_EXE    | [Bitmap Graphic Draw](#0xd2004c-0xd20068-bitmap-graphic-draw) |

Please note that access to the VDP register must always be 4-byte aligned.

### 0xD20000: Skip Screen Update

Setting a value other than zero to this register will skip the screen update every frame (60fps).

### 0xD20004: Sprite Position

Specify the BG layer on which to display the sprite, within the range of 0 to 3.

- 0: Sprites are displayed above BG0 and below BG1 through BG3.
- 1: Sprites are displayed above BG1 and below BG2 through BG3.
- 2: Sprites are displayed above BG2 and below BG3.
- 3: Sprites are displayed above BG3

### 0xD20008-0xD20024: Hardware Scroll

The hardware scroll register behaves differently in Character Pattern Mode and Bitmap Mode.

#### (for Character Pattern Mode)

The VGS-X has a virtual display of 2048x2048 pixels for each BG plane.

For each BG plane, the SX and SY coordinates can be specified within the range 0 to 2047 to define the display origin at the top-left corner.

#### (for Bitmap Mode)

- Writing a positive number to `SX` scrolls the screen **right** by the specified number of pixels.
- Writing a negative number to `SX` scrolls the screen **left** by the specified number of pixels.
- Writing a positive number to `SY` scrolls the screen **down** by the specified number of pixels.
- Writing a negative number to `SY` scrolls the screen **upward** by the specified number of pixels.
- The area after scrolling will be cleared to zero.

### 0xD20028-0xD20034: Bitmap Mode

The VGS-X's display mode defaults to character pattern mode, but can be switched to bitmap mode by setting the BMP register (0xD20028-0xD20034).

- 0: Character Pattern Mode (default)
- 1: Bitmap Mode

When set to Bitmap mode, the name table corresponds to the pixels on the screen (320x200).

Each pixel is set in RGB888 format.

When the pixel color is 0x00000000, it becomes transparent.

### 0xD20038-0xD20048: Clear Screen

You can delete all BGs or specific BGs in bulk.

> Note that when the value to be cleared in bulk is 0, the process is faster compared to non-zero values.

### 0xD2004C-0xD20068: Bitmap Graphic Draw

You can draw various types of shapes to the BG in [Bitmap Mode](#0xd20028-0xd20034-bitmap-mode).

Drawing processing is executed when the execution identifier is written to `G_EXE`.

| `G_BG` | `G_X1` | `G_Y1` | `G_X2` | `G_Y2` | `G_COL` | `G_OPT` | `G_EXE` | Shape |
|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-:|:-|
|☑︎|☑︎|☑︎|-|-|☑︎|-| `0` | Pixel <sup>*1</sup> |
|☑︎|☑︎|☑︎|☑︎|☑︎|☑︎|-| `1` | Line |
|☑︎|☑︎|☑︎|☑︎|☑︎|☑︎|-| `2` | Box |
|☑︎|☑︎|☑︎|☑︎|☑︎|☑︎|-| `3` | Box Fill |
|☑︎|☑︎|☑︎|-|-|☑︎|☑︎| `4` | CHR <sup>*2</sup> |

Remarks:

1. Reading `G_EXE` allows you to read the color of the pixel drawn at the (`G_X1`, `G_Y1`) position on the background plane specified by `G_BG`.
2. When drawing a character, specify the palette number (0 to 15) in `G_COL` and the pattern number (0 to 65535) in `G_OPT`. Additionally, setting the most significant bit of `G_COL (0x80000000)` draws the transparent color, while resetting it skips drawing the transparent color.

> Please note that character drawing performance is not as good as in [Character Pattern Mode](#0xd20028-0xd20034-bitmap-mode).

## I/O Map

I/O instructions in VGS-X can be executed by performing input/output operations on the memory area from 0xE00000 to 0xEFFFFF.

Note that all addresses and values for I/O instructions must be specified as 32-bit values.

| Address  | In  | Out | Description |
|:--------:|:---:|:---:|:------------|
| 0xE00000 |  o  |  -  | [V-SYNC](#0xe00000in---v-sync) |
| 0xE00000 |  -  |  o  | [Console Output](#0xe00000out---console-output) |
| 0xE00004 |  o  |  o  | [Random](#0xe00004io---random) | 
| 0xE01000 |  -  |  o  | [Play VGM](#0xe01000o---play-vgm) |
| 0xE01100 |  -  |  o  | [Play SFX](#0xe01100o---play-sfx) |
| 0xE02000 |  o  |  -  | [Gamepad: D-pad - Up](#0xe200xxi---gamepad) |
| 0xE02004 |  o  |  -  | [Gamepad: D-pad - Down](#0xe200xxi---gamepad) |
| 0xE02008 |  o  |  -  | [Gamepad: D-pad - Left](#0xe200xxi---gamepad) |
| 0xE0200C |  o  |  -  | [Gamepad: D-pad - Right](#0xe200xxi---gamepad) |
| 0xE02010 |  o  |  -  | [Gamepad: A button](#0xe200xxi---gamepad) |
| 0xE02014 |  o  |  -  | [Gamepad: B button](#0xe200xxi---gamepad) |
| 0xE02018 |  o  |  -  | [Gamepad: X button](#0xe200xxi---gamepad) |
| 0xE0201C |  o  |  -  | [Gamepad: Y button](#0xe200xxi---gamepad) |
| 0xE02020 |  o  |  -  | [Gamepad: Start button](#0xe200xxi---gamepad) |
| 0xE7FFFC |  -  |  o  | [Exit](#0xe7fffcout---exit) |

### 0xE00000[in] - V-SYNC

VGS-Zero employed the typical drawing method of the CRT era, namely drawing line by line. However, VGS-X differs significantly from this approach.

When an MC68k program inputs V-SYNC (0xE00000), VGS-X references the VRAM at that point to draw BG0 through BG3 and sprites. It then synchronizes at 60fps before returning a response to the MC68k.

```c
// Execute the process to update the VRAM
drawProc();

// Waiting for vertical sync (Internally executes the InPort at 0xE00000)
vgs_vsync();

// Below processing will be executed after synchronization is complete.
afterDrawProc();
```

The `vgs_vsync` function is defined in [vgs.h](./lib/vgs.h).

> __Design Philosophy:__ By adopting this specification, the VGS-X MC68k eliminates the concept of operating clock frequency. VGS-X can execute MC68k code up to the host computer's maximum performance. You (Game Developers) themselves must describe the minimum spec required to run your game to your customers.

### 0xE00000[out] - Console Output

Writing a value to 0xE00000 allows you to output characters to the console.

This feature is intended for use in game log output and similar applications.

```c
vgs_console_print("Hello, World!\n");
```

The `vgs_console_print` function is defined in [vgs.h](./lib/vgs.h).

### 0xE00004[i/o] - Random

- You can set the seed for random numbers by writing a value to 0xE00004.
- Reading 0xE00004 allows you to obtain a random number (0 to 65535).
- The random number generation in VGS-X guarantees that calling it 65,536 times will return each number from 0 to 65,535 exactly once.

### 0xE01000[o] - Play VGM

Plays the VGM loaded at the index corresponding to the output value.

VGS-X can play VGM data compatible with the following chips (OPN, OPM and SSG) as BGM:

- YM2149 (SSG)
- YM2151 (OPM)
- YM2203 (OPN)
- YM2608 (OPNA) <sup>*1</sup>
- YM2610 (OPNB/OPT)
- YM2612 (OPN2) <sup>*2</sup>

Notes:

1. YM2608 (OPNA) rhythm sound playback is not supported.
2. SN76489 (DCSG) playback is not supported.

We recommend using [Furnace Tracker](https://github.com/tildearrow/furnace) to create VGM data compatible with these FM sound chips.

### 0xE01100[o] - Play SFX

Plays the SFX loaded at the index corresponding to the output value.

The VGS-X ROM cartridge can hold up to 256 .wav files in the following formats.

- Sampling Rate: 44100Hz
- Bit Rate: 16bits
- Number of Channels: 2 (Stereo)

> Please note that while the sound effect functionality is nearly identical to VGS-Zero, the **Number of Channels** differs. (VGS-Zero: 1ch, VGS-X: 2ch)

You can encode to the .wav format compatible with VGS-X by specifying the following options in the `ffmpeg` command:

```bash
ffmpeg -i input.mp3 -acodec pcm_s16le -ar 44100 -ac 2 sfx.wav
```

### 0xE200xx[i] - Gamepad

The VGS-X can capture button inputs from the `D-pad`, `ABXY` buttons, and `Start` button as shown in the diagram below.

![gamepad.png](./gamepad.png)

- When  buttons are pressed, the corresponding input result for 0xE020xx becomes non-zero.
- The cursor keys and left stick are always linked.
- Use the `A` button for confirmation operations.
- Use the `B` button for cancel operations.
- The `X` button is intended for use in operations requiring rapid button presses (e.g., as an attack button in shoot 'em ups).
- The `Y` button is intended for use in auxiliary operations.
- The `Start` button is intended for opening menus or starting games.

The following table shows the button assignments for a typical gamepad:

| VGS-X and XBOX | PC Keybord | Switch | PlayStation | 
|:-:|:-:|:-:|:-:|
|D-pad| Cursor | D-pad | D-pad |
| `A` | `Z` | `B` | `Cross` |
| `B` | `X` | `A` | `Circle` |
| `X` | `A` | `Y` | `Rectangle` |
| `Y` | `S` | `X` | `Triangle` |
| `Start` | `Space` | `Plus` | `Options` |

### 0xE7FFFC[out] - Exit

Issuing an exit request for VGS-X.

In the [Emulator for Debug (SDL2)](#vgs-x-emulator-for-debug), the value written here becomes the process exit code.

# Runtime Library for VGS-X

| Function | Description |
|:---------|:------------|
| `vgs_vsync` | Synchronize the screen output with 60fps |
| `vgs_srand` | Set the random number seed |
| `vgs_rand` | Obtain a 16-bit random value |
| `vgs_rand32` | Obtain a 32-bit random value |
| `vgs_console_print` | Output text to the debug console (no line breaks) |
| `vgs_console_println` | Output text to the debug console (with line breaks) |
| `vgs_d32str` | Convert a 32-bit signed integer to a string |
| `vgs_u32str` | Convert a 32-bit unsigned integer to a string |
| `vgs_put_bg` | Display a character on the BG in [Character Pattern Mode](#0xd20028-0xd20034-bitmap-mode) |
| `vgs_put_bg` | Display a string on the BG in [Character Pattern Mode](#0xd20028-0xd20034-bitmap-mode) |
| `vgs_cls_bg_all` | Clear all BGs |
| `vgs_cls_bg` | Clear a specific BG |
| `vgs_draw_pixel` | Draw a [pixel](#0xd2004c-0xd20068-bitmap-graphic-draw) on the BG in [Bitmap Mode](#0xd20028-0xd20034-bitmap-mode) |
| `vgs_draw_line` | Draw a [line](#0xd2004c-0xd20068-bitmap-graphic-draw) on the BG in [Bitmap Mode](#0xd20028-0xd20034-bitmap-mode) |
| `vgs_draw_box` | Draw a [rectangle](#0xd2004c-0xd20068-bitmap-graphic-draw) on the BG in [Bitmap Mode](#0xd20028-0xd20034-bitmap-mode) |
| `vgs_draw_boxf` | Draw a [filled-rectangle](#0xd2004c-0xd20068-bitmap-graphic-draw) on the BG in [Bitmap Mode](#0xd20028-0xd20034-bitmap-mode) |
| `vgs_draw_character` | Draw a [character-pattern](#character-pattern) on the BG in [Bitmap Mode](#0xd20028-0xd20034-bitmap-mode) |
| `vgs_sprite` | Set [OAM](#oam-object-attribute-memory) attribute values in bulk |
| `vgs_bgm_play` | Play [background music](#0xe01000o---play-vgm) |
| `vgs_sfx_play` | Play [sound effect](#0xe01100o---play-sfx) |
| `vgs_exit` | Exit process |

For detailed specifications, please refer to [./lib/vgs.h](./lib/vgs.h).

Since each function specification is documented in Doxygen format within [./lib/vgs.h](./lib/vgs.h), entering the function name in a code editor like Visual Studio Code with a properly configured C/C++ plugin will trigger appropriate specification suggestions.

# Toolchain

| Name | Description |
|:-----|:------------|
| [vgsx](#vgs-x-emulator-for-debug) | VGS-X Emulator for Debug |
| [bin2var](#bin2var) | Convert binary files to C language code |
| [bmp2chr](#bmp2chr) | Make [CHR](#character-pattern) data from .bmp file |
| [bmp2pal](#bmp2pal) | Make initial [palette](#palette) from .bmp file |
| [makerom](#makerom) | Make ROM file from Program and Assets |
| [vgmplay](#vgmplay) | Play a .vgm file from the command line |

## VGS-X Emulator for Debug

Path: [./tools/sdl2/](./tools/sdl2/)

This is a VGS-X emulator built using SDL2.

It is primarily provided for debugging purposes during game development.

```
usage: vgsx [-g /path/to/pattern.chr]
            [-c /path/to/palette.bin]
            [-b /path/to/bgm.vgm]
            [-s /path/to/sfx.wav]
            [-x expected_exit_code]
            { /path/to/program.elf | /path/to/program.rom }
```

- The `-g`, `-b`, and `-s` options can be specified multiple times.
- Program file (`.elf`) or ROM file (`rom`) are automatically identified based on the header information in the file header.
- The `-x` option is intended for use in testing environments such as CI. If the exit code specified by the user program matches the expected value, the process exits with 0; otherwise, it exits with -1. When this option is specified, SDL video and audio output is skipped.

## bin2var

Path: [./tools/bin2var](./tools/bin2var/)

Converts a binary file into program code for a const uint8_t array in C language.

For example, it comes in handy when you want to use files such as stage map data, scenario scripts and text, or proprietary image formats within your program.

```
bin2var /path/to/binary.rom
```

## bmp2chr

Path: [./tools/bmp2chr](./tools/bmp2chr/)

Generates VGS-X [Character Pattern](#character-pattern) data from 256-color .bmp (Windows Bitmap) file.

```
usage: bmp2chr input.bmp output.chr
```

Remarks:

- The height and width of the image must be 8 or more and a multiple of 8.
- Read the character pattern in 8x8-pixel blocks from top-left to bottom-right.

## bmp2pal

Path: [./tools/bmp2pal](./tools/bmp2pal/)

Generates initial [Palette](#palette) data for VGS-X from 256-color .bmp (Windows Bitmap) file.

```
usage: bmp2pal input.bmp palette.dat
```

## makerom

Path: [./tools/makerom](./tools/makerom/)

Generates a ROM file that combines the program and assets into a single file.

```
usage: makerom  -o /path/to/output.rom
                -e /path/to/program.elf
               [-g /path/to/pattern.chr]
               [-c /path/to/palette.bin]
               [-b /path/to/bgm.vgm]
               [-s /path/to/sfx.wav]
```

Remarks:

- The `-g`, `-b`, and `-s` options can be specified multiple times.
- Files are read sequentially from the specified file.
- The character pattern specified with the first `-g` option is loaded at index 0, and the index of the pattern specified with the second `-g` option is the next one.

## vgmplay

Play a .vgm file from the command line.

```
usage: vgmplay /path/to/bgm.vgm
```

# License

- MC680x0 Emulator - [Musashi](https://github.com/kstenerud/Musashi)
  - Copyright © 1998-2001 Karl Stenerud
  - License: [MIT](./LICENSE-Musashi.txt)
- FM Sound Chip Emulator - [ymfm](https://github.com/aaronsgiles/ymfm)
  - Copyright (c) 2021, Aaron Giles
  - License: [3-clause BSD](./LICENSE-ymfm.txt)
- [VGS-X](https://github.com/suzukiplan/vgsx)
  - Copyright (c) 2025 Yoji Suzuki.
  - License: [MIT](./LICENSE-VGSX.txt)
