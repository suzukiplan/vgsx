# VGS-X

## WIP

This project is still in the WIP; _Work In Progress_ phase.

Please note that the current specifications are subject to frequent, casual breaking changes.

Once this project reaches a stable phase, version 1.0.0 is scheduled for release.

## About VGS-X

- A 16-bit game console featuring an MC68000 compatible CPU and a proprietary VDP.
- Games can be developed using the Programming Language C.
- Supported development environment operating systems are **Ubuntu Linux** or **macOS**. _(Please use the WSL2 if you need Windows development environment.)_

## Setup Build Environment

Since VGS-X can execute MC68000 ELF format modules, you must install `m68k-elf-gcc` to develop games for VGS-X.

On macOS, it can be easily installed via Homebrew. However, since no apt package is provided for Ubuntu Linux, you will need to build it yourself.

### Setup Build Environment: macOS

Please install `m68k-elf-gcc` in an environment where Xcode and Homebrew are installed.

```bash
brew install m68k-elf-gcc
```

### Setup Build Environment: Ubuntu Linux

Below are the installation steps required for game development for the VGS-X.

```bash
# Install Dependencies
sudo apt update
sudo apt install build-essential bison flex libgmp-dev libmpc-dev libmpfr-dev texinfo libncurses5-dev

# Make a work directory for build
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

# Architecture Reference Manual

The following sections provide technical information useful for programming with VGS-X.

## Memory Map

In VGS-X, the first 12MB (0x000000 ~ 0xBFFFFF) of the MC68000's 24-bit (16MB) address space is allocated for programs.

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

[Character patterns](#character-pattern) and Sound data are stored in Read Only Memory, which cannot be directly referenced from the program. Like VGS-Zero, it is specified by pattern number.

## Program

The program area (0x000000 ~ 0xBFFFFF) contains the [ELF32 _(Executable and Linkable Format 32bit)_](https://refspecs.linuxfoundation.org/elf/gabi4+/ch4.eheader.html) binary module.

When tune on the VGS-X, it begins executing the program from the entry point specified in the ELF header of the program loaded from the ROM cartridge.

Note that even if you wish to write your program solely in MC68k assembly language, you must always specify the ELF32 header and Program header that contains valid entry point and executable text.

> However, since there is no advantage to writing high-performance programs in assembly language on the VGS-X, we generally do not recommend writing programs entirely in assembly language.

The following command line the compilation options that must be specified when outputting programs that can run on VGS-X using m68k-elf-gcc.

```
m68k-elf-gcc
    -I${VGSX_ROOT}/lib           ... Specify the VGS-X API header path using the -I option
    -o program                   
    program.c                    
    -L${VGSX_ROOT}/lib           ... Specify the VGS-X API archive path using the -L option
    -T${VGSX_ROOT}/lib/linker.ld ... Specify the linker.ld file describing the VGS-X memory map using the -T option.
    -Wl,-ecrt0                   ... Specify crt0 as the entry point when using the VGS-X runtime
```


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
- Each palette can contain 16 colors in RGB555 format
- For FG and sprites, color number 0 is the transparent color

| Address             | Palette Number | Color Number |
|:-------------------:|:--------------:|:------------:|
| 0xD10000 ~ 0xD10001 |        0       |        0     |
| 0xD10002 ~ 0xD10003 |        0       |        1     |
| 0xD10004 ~ 0xD10005 |        0       |        2     |
|          :          |        :       |        :     |
| 0xD1001A ~ 0xD1001B |        0       |       13     |
| 0xD1001C ~ 0xD1001D |        0       |       14     |
| 0xD1001E ~ 0xD1001F |        0       |       15     |
| 0xD10020 ~ 0xD10021 |        1       |        0     |
| 0xD10022 ~ 0xD10023 |        1       |        1     |
| 0xD10024 ~ 0xD10025 |        1       |        2     |
|          :          |        :       |        :     |
| 0xD101FA ~ 0xD101FB |       15       |       13     |
| 0xD101FC ~ 0xD101FD |       15       |       14     |
| 0xD101FE ~ 0xD101FF |       15       |       15     |

Remarks:

- This table layout is compatible with VGS-Zero.
- 0xD10200 ~ 0xD1FFFF is a mirror of 0xD10000 ~ 0xD101FF (512 bytes).

## Name Table

- The Name Table is a 256x256 x 4bytes two-dimensional array. 
- By setting character patterns and attribute data to it, graphics can be displayed on the background layer.
- The Name Table has a four-layer structure, with BG1 displayed on top of BG0, BG2 on top of BG1, BG3 on top of BG2, and BG4 on top of BG3.
- BG1 ~ BG3 display character pattern color number 0 as transparent, while only BG0 draws the color from color number 0.

| Address             | Size  | Name Table |
|:-------------------:|:-----:|:----------:|
| 0xC00000 ~ 0xC3FFFF | 256KB |     BG0    |
| 0xC40000 ~ 0xC7FFFF | 256KB |     BG1    |
| 0xC80000 ~ 0xCBFFFF | 256KB |     BG2    |
| 0xCC0000 ~ 0xCFFFFF | 256KB |     BG3    |

The bit layout for each element (4 bytes) in the Name Table is as follows:

|  Bit  | Mnemonic | Description |
|:-----:|:--------:|:------------|
|   0   |   F/H    | Flip Horizontal |
|   1   |   F/V    | Flip Vertical |
|  2~7  | reserved | Specify 0 to maintain future compatibility. |
| 12~15 |   PAL    | [Palette](#palette) Number (0~15) |
| 16~31 |   PTN    | [Character Pattern](#character-pattern) Number (0~65535) |

> The 256x256 (2048x2048 pixels) size may be slightly excessive for the VGS-X display resolution (320x200 pixels), but using this size enables future support for bitmap format VRAM.
>
> - 256x256x4 = 262,144 bytes
> - 320x200x2 = 128,000 bytes (minimum size required to display Bitmap format)

## OAM (Object Attribute Memory)

(TODO)

## VDP Register

(TODO)

## I/O Map

I/O instructions in VGS-X can be executed by performing input/output operations on the memory area from 0xE00000 to 0xEFFFFF.

Note that all addresses and values for I/O instructions must be specified as 32-bit values.

| Address  | In  | Out | Description |
|:--------:|:---:|:---:|:------------|
| 0xE00000 |  o  |  -  | [V-SYNC](#0xe00000in---v-sync) |
| 0xE00000 |  -  |  o  | [Console Output](#0xe00000out---console-output) |

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

## Background Music

(TODO)

## Sound Effect

(TODO)

## Gamepad

(TODO)

# License

- MC680x0 Emulator - Musashi
  - Copyright © 1998-2001 Karl Stenerud
  - License: [MIT](./LICENSE-Musashi.txt)
- FM Sound Module Emulator - ymfm
  - Copyright (c) 2021, Aaron Giles
  - License: [3-clause BSD](./LICENSE-ymfm.txt)
- VGS-X Emulator Core Module and Runtime for MC68000
  - Copyright (c) 2025 Yoji Suzuki.
  - License: [MIT](./LICENSE-VGSX.txt)
