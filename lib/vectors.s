.section .vectors,"ax"
.global __vgs_vectors

.set RAM_BASE, 0x00F00000
.set RAM_SIZE, 0x00100000
.set RAM_TOP, RAM_BASE + RAM_SIZE

.macro VGS_VECTOR handler, count=1
.rept \count
.long \handler
.endr
.endm

__vgs_vectors:
.long RAM_TOP - 4   /* Initial SP (top of WRAM) */
.long crt0         /* Reset PC */
VGS_VECTOR _bus_error
VGS_VECTOR _address_error
VGS_VECTOR _illegal, 1
VGS_VECTOR _zero_div
VGS_VECTOR _chk_inst
VGS_VECTOR _trapv

/* Fill remaining vectors with _illegal to catch unexpected traps. */
VGS_VECTOR _illegal, (256 - 8)
