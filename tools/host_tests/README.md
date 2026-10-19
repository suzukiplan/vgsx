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
