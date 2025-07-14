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
nop
nop
nop
addi x1, x0, 7 ; LED value
lui x2, 0xc0010 ; LED address
addi x2, x2, 0xe00 ;
sw x1, 0x0(x2) ; set LED

;loop valuable
lui x5, 01000 ; loop max
;addi x5, x0, 0x10

lui x6, 0x00001 ;
lui x7, 0x00002 ;
lui x8, 0x00003 ;
lui x9, 0x00004 ;
lui x10, 0x00005 ;

sw x6, 0x0(x0)
sw x7, 0x4(x0) 
sw x8, 0x8(x0)
sw x9, 0xC(x0)
sw x10, 0x10(x0)
;nop

lw x3, 0x0(x0)
bne x3, x6, label_fail
addi x1, x0, 6 ; LED value
sw x1, 0x0(x2) ; set LED

lw x3, 0x4(x0)
bne x3, x7, label_fail
addi x1, x0, 5 ; LED value
sw x1, 0x0(x2) ; set LED

lw x3, 0x8(x0)
bne x3, x8, label_fail
addi x1, x0, 4 ; LED value
sw x1, 0x0(x2) ; set LED

lw x3, 0xC(x0)
bne x3, x9, label_fail
addi x1, x0, 3 ; LED value
sw x1, 0x0(x2) ; set LED

lw x3, 0x10(x0)
bne x3, x10, label_fail
addi x1, x0, 2 ; LED value
sw x1, 0x0(x2) ; set LED

jalr x0, x0, label_pass
nop
nop
nop
nop
nop



:label_fail
addi x1, x0, 1 ; LED value
;sw x1, 0x0(x2) ; set LED
jalr x0, x0, label_fail
nop
nop


:label_pass
;lui x2, 01000 ; loop max
addi x2, x0, 0x10
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
