# Changes

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
