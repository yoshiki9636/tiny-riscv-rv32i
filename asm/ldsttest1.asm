;*
;* My RISC-V RV32I CPU
;*   Test Code : LED Chika Chika
;*    RV32I code
;* @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
;* @copylight	2021 Yoshiki Kurokawa
;* @license		https://opensource.org/licenses/MIT     MIT license
;* @version		0.1
;*

nop
nop
addi x1, x0, 7 ; LED value
lui x2, 0xc0010 ; LED address
addi x2, x2, 0xe00 ;
sw x1, 0x0(x2) ; set LED

lui x3, 0x00001 ; value A
lui x4, 0x00002 ; value B
lui x5, 0x00004 ; address 0x4000
lui x6, 0x00008 ; address 0x8000

sw x3, 0x0(x5) ; store value A to (0x4000)
sw x4, 0x0(x6) ; store value B to (0x8000)
nop
nop
nop
lw x7, 0x0(x5) ; load A
lw x8, 0x0(x6) ; load B
lw x9, 0x0(x5) ; load A
lw x10, 0x0(x6) ; load B
lw x11, 0x0(x5) ; load A
lw x12, 0x0(x6) ; load B
;lw x13, 0x0(x5) ; load A
;lw x14, 0x0(x6) ; load B

bne x7, x3, label_fail
bne x8, x4, label_fail
bne x9, x3, label_fail
bne x10, x4, label_fail
bne x11, x3, label_fail
bne x12, x4, label_fail
;bne x13, x3, label_fail
;bne x14, x4, label_fail
jalr x0, x0, label_pass

:label_fail
addi x1, x0, 1 ; LED value
sw x1, 0x0(x2) ; set LED
jalr x0, x0, label_fail
nop
nop

:label_pass
lui x2, 01000 ; loop max
;addi x2, x0, 0x10
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
