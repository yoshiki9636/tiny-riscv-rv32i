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
lui x2, 1000 ; loop max
and x3, x0, x3 ; LED value
lui x4, 0xc0010 ; LED address
addi x4, x4, 0xe00 ;
and x5, x0, x5 ; 
lui x6, 0x08000 ; dram address max
:label_led
and x1, x0, x1 ; loop counter
:label_waitloop
addi x1, x1, 1 
blt x1, x2, label_waitloop
lh x3, 0x0(x5)
addi x5, x5, 0x2
sw x3, 0x0(x4)
bne x5, x6, label_led
and x5, x0, x5 ; 
jalr x0, x0, label_led
nop
nop
nop
nop

