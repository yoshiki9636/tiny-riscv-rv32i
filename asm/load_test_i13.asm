;*
;* My RISC-V RV32I CPU
;*   Test Code Load Instructions : No.13
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
lui x9, 0x1 ; offset

; store data for test
lui x3, 0x76543 ;
ori x3, x3, 0x210 ; test value (1)
sw x3, 0x0(x9)
sw x3, 0x8(x9)
lui x3, 0xfedcb ;
ori x3, x3, 0x298 ; test value (2)
lui x4, 0x00001
srli x4, x4, 1;
or x3, x3, x4
sw x3, 0x4(x9)
sw x3, 0xc(x9)
; test lb offset 0
:fail_test1
lui x4, 0x1 ; offset
lb x5, 0x0(x4)
ori x6, x0, 0x010
bne x5, x6, fail_test1
; next value
addi x1, x0, 6 ; LED value
sw x1, 0x0(x2) ; set LED
; test lb offset 1
:fail_test2
lui x4, 0x1 ; offset
ori x4, x4, 0x005
lbu x5, 0x0(x4)
ori x6, x0, 0x0ba
bne x5, x6, fail_test2
; next value
addi x1, x0, 5 ; LED value
sw x1, 0x0(x2) ; set LED
; test lb offset 2
:fail_test3
lui x4, 0x1 ; offset
ori x4, x4, 0x006
lbu x5, 0x0(x4)
ori x6, x0, 0x0dc
bne x5, x6, fail_test3
; next value
addi x1, x0, 4 ; LED value
sw x1, 0x0(x2) ; set LED
; test lb offset 3
:fail_test4
lui x4, 0x1 ; offset
ori x4, x4, 0x003
lbu x5, 0x0(x4)
ori x6, x0, 0x076
bne x5, x6, fail_test4
; next value
addi x1, x0, 3 ; LED value
sw x1, 0x0(x2) ; set LED
; test lh offset 0
:fail_test5
lui x4, 0x1 ; offset
ori x4, x4, 0x004
lhu x5, 0x0(x4)
lui x6, 0xb
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
lhu x5, 0x0(x4)
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
lui x2, 10 ; loop max
;ori x2, x0, 10
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
