.cpu cortex-a53
.data
.text
.align 3
.global   main
.type    main, %function
main:
mov   x19, x8
movz  x8, #0
mov   x19, x8
mov   x8, x19
mov   x19, #0
movz  x8, #0
movk  x8, #0, lsl #16
fmov  s16, w19
sub    x8, sp, #1040
mov   sp, x8
str   x30, [sp, #1032]
main_entry:
add    x8, sp, #0
mov   x28, x8

mov   x19, x9
movz  x9, #7
mov   x8, x9
mov   x9, x19
mov   x19, #0
str   x8, [x28]

ldr   x28, [x28]

add    x9, sp, #16
mov   x8, x9
stp x0, x1, [x8], #16
stp x2, x3, [x8], #16
stp x4, x5, [x8], #16
stp x6, x7, [x8], #16
stp x19, x20, [x8], #16
stp x21, x22, [x8], #16
stp x23, x24, [x8], #16
stp x25, x26, [x8], #16
stp x27, x28, [x8], #16
stp x29, x30, [x8], #16
stp s0, s1, [x8], #8
stp s2, s3, [x8], #8
stp s4, s5, [x8], #8
stp s6, s7, [x8], #8
stp s16, s17, [x8], #8
stp s18, s19, [x8], #8
stp s20, s21, [x8], #8
stp s22, s23, [x8], #8
stp s24, s25, [x8], #8
stp s26, s27, [x8], #8
stp s28, s29, [x8], #8
stp s30, s31, [x8], #8
mov   x0, x28
bl    putint
add    x9, sp, #16
mov   x8, x9
ldp x0, x1, [x8], #16
ldp x2, x3, [x8], #16
ldp x4, x5, [x8], #16
ldp x6, x7, [x8], #16
ldp x19, x20, [x8], #16
ldp x21, x22, [x8], #16
ldp x23, x24, [x8], #16
ldp x25, x26, [x8], #16
ldp x27, x28, [x8], #16
ldp x29, x30, [x8], #16
ldp s0, s1, [x8], #8
ldp s2, s3, [x8], #8
ldp s4, s5, [x8], #8
ldp s6, s7, [x8], #8
ldp s16, s17, [x8], #8
ldp s18, s19, [x8], #8
ldp s20, s21, [x8], #8
ldp s22, s23, [x8], #8
ldp s24, s25, [x8], #8
ldp s26, s27, [x8], #8
ldp s28, s29, [x8], #8
ldp s30, s31, [x8], #8

mov   x19, x8
movz  x8, #0
mov   x0, x8
mov   x8, x19
mov   x19, #0
ldr   x30, [sp, #1032]
add    x8, sp, #1040
mov   sp, x8
ret   



