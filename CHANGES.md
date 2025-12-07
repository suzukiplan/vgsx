# Changes

## Next Version

The current `master` branch is in a nightly state.

Pending fixes:

- Added `VGSX::setSaveDataDirectory` method
- Added `VGSX::setConsoleCallback` method
- `VGSX::setLogCallback` changes to `std::function`
- Changed the master volume range: 0-256 -> 0-255

## Version 0.24.0 **(Disruptive)**

- Support 4k Display
- Added Calendar API
- Added Software Reset API
- Added `slx` and `sly` (Scale Lock X/Y) to OAM.
- Added `pri` (High Priority Flag) to OAM.
- Added `vgs_sprite_alpha8` function.
- Added `vgs_abort` function.
- Changed the behavior when `OAM.alpha` is set to `0` from “alpha disabled” to “fully transparent.” **(Disruptive)**
- Changed the behavior when `OAM.scale` is set to `0` from “scaling disabled” to “scaled to 0%.” **(Disruptive)**
- Changed the default values when initializing a sprite with `vgs_sprite` to `OAM.alpha = 0xFFFFFF` and `OAM.scale = 100`.

## Version 0.23.0

- Added a Japanese Manual (README-jp.md)

## Version 0.22.0

- optimize & refactor the VDP implementation
- optimize the runtime speed of the Musashi (MC68k emulator)
- optimize the runtime speed of the ymfm (YM2612 emulator)
- code refactor

## Version 0.21.0

Implementing Musashi as a single header.

## Version 0.20.0 **(Disruptive)**

The FM sound chip specification has been changed to **support only the YM2612**.

## Version 0.19.1

- Tools: Fixed startup reset omission

## Version 0.19.0

- Core: Changed the specification to initialize character patterns and palettes to their default state when resetting the VDP.
- CRT: Added the cg functions: `vgs_pal_get` and `vgs_pal_set`

## Version 0.18.0

- Core+CRT: Implemented User-Defined I/O.
- Fixed an issue where the pitch of the music track was incorrect.

## Version 0.17.0

- Core: Update Boot Bios
- Core: Improve rotation rendering performance
- CRT: Eliminate the risk of platform differences occurring between `vgs_sin` and `vgs_cos`
- CRT: Renamed a function: `vgs_copy_ptn` -> `vgs_ptn_copy` **(Disruptive)**
- CRT: Change `vgs_put_bg` to an inline function.

## Version 0.16.0

- Core: Fixed a bug where the ELF32 NOBITS section header was expanded incorrectly, causing a buffer overflow.
- Core: Update Boot Bios
- Core: Added the Copy Character Pattern function.
- CRT: Added a CG function: `vgs_copy_ptn`

## Version 0.15.0

- Core+CRT: Added the boot BIOS
- Core+CRT: Added the BGM pause, resume and fadeout functions
- Core+CRT: Added a SFX stop function
- Core+CRT: Added functionality to set and get the Master Volume for BGM and SFX respectively.
- Core+CRT: Added a Window function (`vgs_draw_window`)
- Core+CRT: Added a Clear function (`vgs_draw_clear`)
- CRT: Change the argument specification for `vgs_draw_box` and `vgs_draw_boxf` from (x1,y1,x2,y2) to (x,y,width,height) **(Disruptive)**
- Fixed an issue where Alpha Blend did not function as intended.

## Version 0.14.0

- Core+CRT: Supports Alpha Blending for Sprites. (`OAM::alpha`)
- Core+CRT: Supports Mask for Sprites. (`OAM::mask`)

## Version 0.13.0

- Core: Added DMA functionality to convert Japanese UTF-8 strings to SJIS.
- CRT: Added a string fcuntion: `vgs_sjis_from_utf8`
- Example: Changed the source code for `example/06_k8x12` from SJIS to UTF-8 and modified it to use the conversion function.

## Version 0.12.0

Supports Japanese display using k8x12.

## Version 0.11.0

- Core: Added the Propotional Font functions
- CRT: Added the bitmap functions: `vgs_pfont_init`, `vgs_pfont_get`, `vgs_pfont_set`, `vgs_pfont_print` and `vgs_pfont_strlen`
- bugfix: The option to not draw the number 0 in `vgs_draw_character` does not render as intended.

## Version 0.10.0

- Fixed a critical bug related to DMA.
- Core: Implemented Gamepad's Type and Name interfaces.
- Core: Added the Skip Rendering a Specific BG function.
- CRT: Added a function: `vgs_skip_bg`
- CRT: Added a bitmap function: `vgs_read_pixel`
- CRT: Added a sprite function: `vgs_sprite_hide_all`
- CRT: Added the functions: `vgs_scroll`,  `vgs_scroll_x` and  `vgs_scroll_y`
- CRT: Added the string functions: `vgs_strcpy` and `vgs_strcat`
- CRT: Added the gamepad functions.

## Version 0.9.0

- CRT: Renamed the OAM structure to ObjectAttributeMemory to avoid name conflicts with `#define` literals. **(Disruptive)**
- CRT: Added the sprite functions: `vgs_sprite_priority` and `vgs_oam`
- CRT: Added a bitmap function: `vgs_draw_mode`
- CRT: Added the getting screen width functions: `vgs_bg_width`,  `vgs_bg_height`,  `vgs_chr_width`, `vgs_chr_width`, `vgs_draw_width` and `vgs_draw_height`
- Toolchain: Supports 16-bit format output with `bin2var`.
- Toolchain: Added `csv2var`, a tool to convert Tiled Map Editor CSV files.
- Toolchain: Place the license file using the `makeprj` command.
- Toolchain: The `makeprj` command now supports specifying relative paths.
- Toolchain: Perform verification before executing processing with the `makeprj` command.
- Toolchain: Remove the `name` option from the `makeprj` command. **(Disruptive)**

## Version 0.8.0

- CRT: Added a math function: `vgs_hitchk`
- Toolchain: Prevent specifying relative paths with `makeprj`
- Toolchain: Modified the project created with the `makeprj` command to initialize the Git submodule every time it is built.
- Toolchain: Prevent the `vgmplay` command from being built every time (to reduce build time when creating initial projects)

## Version 0.7.0

- Toolchain: added a `makeprj` command.
- Document: How to Create a New Project

## Version 0.6.0

- Core: Correct Logging System (do not stdout in the core)
- Example: added a `03_rotate`.
- Toolchain: Implemented an enhancement to allow specifying multiple files in the format `-g file1 file2 file3` using the `-g`, `-b`, and `-s` options of the makerom command.
- Toolchain: Fixed the SDL2 emulator to display the number of characters loaded from the ROM.
- Toolchain: Fixed an issue where values of 0x80 or higher were incorrectly displayed in the SDL2 emulator's file dump.
- Toolchain: Added a reset function via the R key to the SDL2 version emulator.

## Version 0.5.0

- Core: Implemented SaveData functions
- Core: Implemented Large Sequencial File I/O functions
- CRT: Added the save functions: `vgs_save`, `vgs_load` and `vgs_save_check`
- CRT: Added the save functions: `vgs_seq_open_w`, `vgs_seq_write`, `vgs_seq_commit`, `vgs_seq_open_r` and `vgs_seq_read`
- CRT: Added the i-math inline functions: `vgs_abs` and `vgs_sgn`
- CRT: Added a string function: `vgs_stricmp`
- CRT: Added a ctype function: `vgs_atoi`

## Version 0.4.0

- Core: Implemented IMATH functions
- CRT: Added `vgs_degree`, `vgs_sin` and `vgs_cos`

## Version 0.3.0

- Core: Implemented DMA functions
- CRT: Separate logging APIs from `libc.a` to `liblog.a` **(Disruptive)**
- CRT: Added `vgs_memcpy`, `vgs_memset` and `vgs_strlen` _(Using DMA)_
- CRT: Added `vgs_strcmp`, `vgs_strncmp`, `vgs_strstr`, `vgs_strchr` and `vgs_strrchr` _(Not DMA)_
- CRT: Added `vgs_isdigit`, `vgs_isupper`, `vgs_islower`, `vgs_isalpha`, `vgs_isalnum`, `vgs_toupper` and `vgs_tolower`
- Other: Preparing for Unit Testing in CI

## Version 0.2.0

- Core: Fixed an issue where audio noise might be played.
- Core: Fixed memory leak during VGM playback.
- CRT: Changed specification to omit pointer operators for integer register access. **(Disruptive)**
- CRT: Creation and review of CRT specification documents **(Disruptive)**
- Tools: Add the tools: `bin2var`, `vgmplay`
- Changed directory structure: `src/sdl2` -> `tools/sdl2` and `src/core/*` -> `src/*`

## Version 0.1.0

- First beta version.
- The state in which it has been determined that development of the launch title is feasible.
- All subsequent modifications will be made via Pull Requests.
- Until the transition to Version 1.0.0, significant specification changes, including breaking changes, may occur.
- We plan to transition to version 1.0.0 between the completion of the launch title and its release.
