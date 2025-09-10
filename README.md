# VGS-X

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
cd ~/m68k-work

# Build and Install the binutils for MC68k
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

> We referenced the following article.
>
> [https://computeralgebra.hatenablog.com/entry/2025/02/26/233231](https://computeralgebra.hatenablog.com/entry/2025/02/26/233231).
>
> We would like to thank the author for writing it!

## Build and Execute an Example

If you've finished installing `m68k-elf-gcc`, you're now ready to start developing games for the VGS-X.

The following steps show how to obtain this repository using `git clone` and then run the example that displays “HELLO, WORLD!”.

```bash
git clone https://github.com/suzukiplan/vgs-x
cd vgs-x/example/01_hello
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

The program area (0x000000 ~ 0xBFFFFF) contains the ELF _(Executable and Linkable Format)_ binary module.

When power is applied to the VGS-X, it begins executing the program from the entry point specified in the ELF header of the program loaded from the ROM cartridge.

Even if you wish to write your program solely in MC68k assembly language, you must always specify the entry point in the ELF header.

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

## Palette

## Name Table

## OAM (Object Attribute Memory)

## VDP Register

## I/O Map

## Background Music

## Sound Effect

## Gamepad
