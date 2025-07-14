;*
;* My RISC-V RV32I CPU
;*   Test Code D$ test 1
;*    RV32I code
;* @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
;* @copylight	2024 Yoshiki Kurokawa
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

; store pattern data for test
add x4, x0, x0; address 0x0
lui x3, 0xa5a5a ;
addi x3, x3, 0x5a5 ; test value (1)
sw x3, 0x0(x4)
addi x4, x4, 0x10
sw x3, 0x0(x4)
addi x4, x4, 0x10
sw x3, 0x0(x4)
addi x4, x4, 0x10
sw x3, 0x0(x4)

; clear LED to black
addi x1, x0, 6 ; LED value
sw x1, 0x0(x2) ; set LED

; store pattern data for test
lui x6, 0x00100;
lui x5, 0xdeadc ;
addi x5, x5, 0xeef ; test value (1)
sw x5, 0x0(x6)
addi x6, x6, 0x10
sw x5, 0x0(x6)

; clear LED to black
addi x1, x0, 5 ; LED value
sw x1, 0x0(x2) ; set LED

:fail_test1
; load from 0x0 to check correct data
add x4, x0, x0; address 0x0
lw x7, 0x0(x4)
bne x3, x7, fail_test1
addi x4, x4, 0x10
lw x7, 0x0(x4)
bne x3, x7, fail_test1


; test finished
nop
nop
;lui x2, 01000 ; loop max
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
