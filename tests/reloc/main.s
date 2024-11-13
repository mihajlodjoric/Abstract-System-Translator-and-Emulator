.global a
.extern c, d
.section text
ld a, %r1
ld b, %r2
ld c, %r3
ld d, %r4
halt
.section data
a: .word 0x1
b: .word 0x2
.end

