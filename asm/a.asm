;*
;* My RISC-V RV32I CPU
;*   Test Code Load Instructions : No.5
;*    RV32I code
;* @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
;* @copylight	2021 Yoshiki Kurokawa
;* @license		https://opensource.org/licenses/MIT     MIT license
;* @version		0.1
;*

nop
nop
; clear LED to black
addi x1, x0, 7 ; LED value
lui x2, 0xc0010 ; LED address
addi x2, x2, 0xe00 ;
sw x1, 0x0(x2) ; set LED

; store data for test
lui x4, 0x1
lui x3, 0x76543 ;
ori x3, x3, 0x210 ; test value (1)
sw x3, 0x0(x4)
sw x3, 0x8(x4)
lui x3, 0xfedcc ;
ori x3, x3, 0xa98 ; test value (2)
sw x3, 0x4(x4)
sw x3, 0xc(x4)

; test load same address
lui x9, 0xc0010 ; tx out
addi x9, x9, 0xc00 ; tx out
ori x7, x0, 0xf
ori x8, x0, 0x30

ori x12, x0, 0x0d
ori x13, x0, 0x0a

:fail_test1
ori x10, x0, 8 ; loop max
ori x11, x0, 0 ; loop counter

:inner_loop1
lw x5, 0x0(x4)
slli x14, x11, 2 ;
srl x5, x5, x14 ;
and x5, x5, x7
or x5, x5, x8
sw x5, 0x0(x9)
addi x11, x11, 1
bne x11, x10, inner_loop1
sw x12, 0x0(x9) ; cr
sw x13, 0x0(x9) ; lf

ori x11, x0, 0 ; loop counter
:inner_loop2
lw x6, 0x0(x4)
slli x14, x11, 2 ;
srl x6, x6, x14 ;
and x6, x6, x7
or x6, x6, x8
sw x6, 0x0(x9)
addi x11, x11, 1
bne x11, x10, inner_loop2

sw x12, 0x0(x9) ; cr
sw x13, 0x0(x9) ; lf
lw x5, 0x0(x4)
lw x6, 0x0(x4)
bne x5, x6, fail_test1

; next value
addi x1, x0, 6 ; LED value
sw x1, 0x0(x2) ; set LED
; test lb offset 1
:fail_test2
lui x4, 0x1 ; offset
ori x4, x4, 0x005
lb x5, 0x0(x4)
ori x6, x0, 0xfba
bne x5, x6, fail_test2
; next value
addi x1, x0, 5 ; LED value
sw x1, 0x0(x2) ; set LED
; test lb offset 2
:fail_test3
lui x4, 0x1 ; offset
ori x4, x4, 0x006
lb x5, 0x0(x4)
ori x6, x0, 0xfdc
bne x5, x6, fail_test3
; next value
addi x1, x0, 4 ; LED value
sw x1, 0x0(x2) ; set LED
; test lb offset 3
:fail_test4
lui x4, 0x1 ; offset
ori x4, x4, 0x003
lb x5, 0x0(x4)
ori x6, x0, 0x076
bne x5, x6, fail_test4
; next value
addi x1, x0, 3 ; LED value
sw x1, 0x0(x2) ; set LED
; test lh offset 0
:fail_test5
lui x4, 0x1 ; offset
ori x4, x4, 0x004
lh x5, 0x0(x4)
lui x6, 0xffffb
ori x6, x6, 0x298
lui x7, 0x00001
srli x7, x7, 1
or x6, x6, x7
bne x5, x6, fail_test5
; next value
addi x1, x0, 2 ; LED value
sw x1, 0x0(x2) ; set LED
; test lh offset 1
:fail_test6
lui x4, 0x1 ; offset
ori x4, x4, 0x002
lh x5, 0x0(x4)
lui x6, 0x00007
ori x6, x6, 0x654
bne x5, x6, fail_test6
; next value
addi x1, x0, 1 ; LED value
sw x1, 0x0(x2) ; set LED
; test lw 
:fail_test7
lui x4, 0x1 ; offset
ori x4, x4, 0x004
lw x5, 0x0(x4)
lui x6, 0xfedcb
ori x6, x6, 0x298
lui x7, 0x00001
srli x7, x7, 1
or x6, x6, x7
bne x5, x6, fail_test7
addi x1, x0, 0 ; LED value
sw x1, 0x0(x2) ; set LED
; next value
addi x1, x0, 0 ; LED value
sw x1, 0x0(x2) ; set LED
; test finished
nop
nop
;lui x2, 10 ; loop max
ori x2, x0, 10
and x3, x0, x3 ; LED value
and x4, x0, x4 ;
lui x4, 0xc0010 ; LED address
addi x4, x4, 0xe00 ;
:label_led
and x1, x0, x1 ; loop counter
:label_waitloop
addi x1, x1, 1
blt x1, x2, label_waitloop
addi x3, x3, 1
sw x3, 0x0(x4)
jalr x0, x0, label_led
nop
nop
nop
nop
