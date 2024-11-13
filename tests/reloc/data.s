.global b, c
.extern a
.section data
b: .word 0x2
c: .word 0x3
d: .word a
.end
