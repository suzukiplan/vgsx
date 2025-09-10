# VGS-X

## About

- MC68000 互換の CPU を搭載した 16bit ゲーム機です
- C言語 を用いてゲームを開発することができます
- 開発環境の対応OSは **Ubuntu Linux** または **macOS** です _（Windowsで開発する必要がある場合はWSL2の利用を推奨 ※未検証）_

## Setup Build Environment

VGS-X は MC68000 の ELF 形式のモジュールを実行できるため、VGS-X 用のゲームを開発するには `m68k-elf-gcc` をインストールする必要があります。

macOS であれば Homebrew で提供されているため手軽にインストールできますが、Ubuntu Linux 用の apt パッケージは提供されていないためビルドをする必要があります。

### Setup Build Environment: macOS

XCODE と Homebrew がインストールされた環境で `m68k-elf-gcc` をインストールしてください。

```bash
brew install m68k-elf-gcc
```

### Setup Build Environment: Ubuntu Linux

[こちらの記事](https://computeralgebra.hatenablog.com/entry/2025/02/26/233231) を参考に構築することができます。

以下、VGS-X 用のゲーム開発に必要なインストール手順のみを抜粋して記します。

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
