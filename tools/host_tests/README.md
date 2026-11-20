# Host tests

Run the rendering and I/O regression tests:

```sh
make -C tools/host_tests
```

Run the sprite rendering benchmark separately:

```sh
make -C tools/host_tests benchmark
```

The benchmark renders 256 40x40 title cards with odd X scales, a locked Y
scale, and varying rotation angles, matching Battle Hanafuda's title effect.
It reports median milliseconds per frame over five runs and a framebuffer
checksum. Compare builds on the same machine with the same compiler and flags;
performance depends on system load. Timing is not a pass/fail test.

The regression tests compare sprite output against direct inverse sampling,
including clipping, flips, scale locks, transparent pixels, opaque palette
black, RAM bitmaps, alpha, and masks over a varying background. They also cover
pivot scaling and Mode 7 independently and together.

Frame pacing tests simulate 60/120 Hz presentation, timer overshoot, and stall
recovery without sleeping or depending on machine speed.

Save I/O tests use isolated temporary directories to cover implicit saves on
exit, destruction and reset, requested sizes for short or missing files,
legacy loads, explicit saves while enabled, disabling, and invalid requests.

BGM tests use synthetic VGM 1.61 streams with 0x61/0x62/0x63 waits,
partial waits, nonzero loop starts, partitioned rendering, concurrent position
reads, and invalid loads. Core I/O tests cover start/restart/change, pause/resume,
stop, mute/zero volume, natural end, fadeout completion, reset, and MMIO reads.
The standalone driver tests run at 22,050, 44,100, and 48,000Hz and verify that
position counts consumed VGM wait units at each rate. The existing driver's
one-wait-unit-per-output-frame scheduling is preserved; this does not introduce
rate conversion for VGM waits at non-44,100Hz rates. The VGS-X core uses 44,100Hz.

With the existing MC68030 toolchain installed, also run:

```sh
make -C tools/host_tests check-m68k
```

This compiles the real library API with `m68k-elf-gcc -m68030 -O2` and runs it
in Musashi, checking the 32-bit MMIO load, fractional double seconds, FPU
conversion, and explicit stop. It needs no target C runtime or new dependency.
