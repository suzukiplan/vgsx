.section .vectors,"ax"
.global __vgs_vectors
__vgs_vectors:
.long 0xF0FFFFFC
.long crt0
.long _bus_error
.long _address_error
.long _illegal
.long _zero_div
.long _chk_inst
.long _trapv
