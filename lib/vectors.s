.section .vectors,"ax"
.global __vgs_vectors

.macro VGS_VECTOR handler, count=1
.rept \count
.long \handler
.endr
.endm

__vgs_vectors:
.long 0xF0FFFFFC   /* Initial SP */
.long crt0         /* Reset PC */
VGS_VECTOR _bus_error
VGS_VECTOR _address_error
VGS_VECTOR _illegal, 1
VGS_VECTOR _zero_div
VGS_VECTOR _chk_inst
VGS_VECTOR _trapv

/* Fill remaining vectors with _illegal to catch unexpected traps. */
VGS_VECTOR _illegal, (256 - 8)
