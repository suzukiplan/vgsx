# VGS-X

## About VGS-X

- A 16-bit game console featuring an MC68000 compatible CPU and a proprietary VDP.
- Games can be developed using the C programming language.
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

You can build it by referring to [this article](https://computeralgebra.hatenablog.com/entry/2025/02/26/233231).

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

The space between the program and WRAM (0xC00000 ~ 0xEFFFFF = 3MB) constitutes the memory map for the VDP and I/O, and these areas are used for VDP access and input/output with external devices.

| Address             | Size    | Description  |
|:-------------------:|--------:|:-------------|
| 0x000000 ~ 0xBFFFFF | 12288KB | Program      |
| 0xC00000 ~ 0xCFFFFF |  1024KB | Name Table   |
| 0xD00000 ~ 0xD0FFFF |    64KB | Palette      |
| 0xD10000 ~ 0xD1FFFF |    64KB | VDP Register |
| 0xD20000 ~ 0xDFFFFF |   896KB | Reserved     |
| 0xE00000 ~ 0xEFFFFF |  1024KB | I/O          |
| 0xF00000 ~ 0xFFFFFF |  1024KB | WRAM         |

Character patterns and Sound data are stored in Read Only Memory, which cannot be directly referenced from the program. Like VGS-Zero, it is specified by pattern number.

## Program

The program area (0x000000 ~ 0xBFFFFF) contains the ELF format executable module.

When power is applied to the VGS-X, it begins executing the program from the entry point specified in the ELF header of the program loaded from the ROM cartridge.

Even if you wish to write your program solely in assembly language, you must always specify the entry point in the ELF header.
