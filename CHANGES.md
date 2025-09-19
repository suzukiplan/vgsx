# Changes

## Version 0.3.0

- Core: Implemented DMA functions
- CRT: Separate log-related APIs from libc.a to liblog.a **(Disruptive)**
- CRT: Added `vgs_memcpy`, `vgs_memset` and `vgs_strlen` _(Using DMA)_
- CRT: Added `vgs_strchr` and `vgs_strrchr` _(Not DMA)_

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
