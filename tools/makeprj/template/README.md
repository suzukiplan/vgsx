# New Game

## Makefile Guide

- This Makefile builds a **VGSX-based your game** using the Motorola 68030 cross compiler toolchain (`m68k-elf-gcc`) and VGS-X tools.  
- It supports automatic dependency generation, separate build directories, and automatic recompilation when headers change.

### 📁 Directory Structure

```
project/
  ├── vgsx/               # VGS-X library and tools (submodule)
  ├── src/                # Source code (.c / .h)
  ├── obj/                # Automatically created for .o and .d files
  ├── bmp/                # Bitmap assets
  ├── vgm/                # BGM data
  ├── wav/                # Sound effects
  └── Makefile
```

### 🔧 How to use

- Source code files (`*.c`, `*.h`) are automatically compiled when added to the `./src` directory.
- After adding image data as 8px-by-8px 16-color or 256-color bitmap files to the `./bmp` directory, add them to `CHR_FILES` variable in the `Makefile`.
- After adding YM2612 `.vgm` files to the `./vgm` directory, add them to the `BGM_FILES` variable in the `Makefile`.
- After adding sound effect data as 44100Hz, 16-bit, stereo `.wav` files to the `./wav` directory, please add them to the `WAV_FILES` variable in the `Makefile`.

## Licenses

This program uses the following OSS.

- [SDL2](https://www.libsdl.org/)
  - Copyright (C) 1997-2025 Sam Lantinga <slouken@libsdl.org>
  - License: [ZLIB License](./LICENSE-SDL2.txt)
- MC680x0 Emulator - [Musashi](https://github.com/kstenerud/Musashi)
  - Copyright © 1998-2001 Karl Stenerud
  - License: [MIT](./LICENSE-Musashi.txt)
- FM Sound Chip Emulator - [ymfm](https://github.com/aaronsgiles/ymfm)
  - Copyright (c) 2021, Aaron Giles
  - License: [3-clause BSD](./LICENSE-ymfm.txt)
- Japanese Font - [k8x12](https://littlelimit.net/k8x12.htm)
  - Created by Num Kadoma
  - License: [Free Software](./LICENSE-k8x12.txt)
- [VGS-X](https://github.com/suzukiplan/vgsx) and VGS Standard Library for MC68030
  - Copyright (c) 2025 Yoji Suzuki.
  - License: [MIT](./LICENSE-VGSX.txt)

> NOTE for Game Developer: Since SDL2 is only used in the debug emulator, you can exclude it if you are developing your own runtime environment without SDL2.
