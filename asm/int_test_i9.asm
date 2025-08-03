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
lui x2, 1 ; loop max
;ori x2, x0, 0x5 ; loop max
and x3, x0, x3 ; LED value
and x4, x0, x4 ; 
addi x5, x0, 7 ; LED mask value
ori x8, x0, 7 ; check value
lui x4, 0xc0010 ; LED address
addi x4, x4, 0xe00 ;
lui x9, 0xc0010 ; global interrupt set enable address
addi x9, x9, 0xa00 ;
addi x10, x0, 0x2 ; enable interrupt pin
sw x10, 0x0(x9)
ori x7, x0, 0x007c ; interrupt tvector address
csrrw x7, 0x305, x7 ; wirte to mtvec
lui x7, 1 ; set meip bit on mie 
ori x7, x0, 0x800 ; set meip bit on mie 
csrrw x7, 0x304, x7 ; wirte to mie
ori x7, x0, 0x8 ; set mie bit on mstatus 
csrrw x7, 0x300, x7 ; wirte to mstatus
:label_led
and x1, x0, x1 ; loop counter
:label_waitloop
addi x1, x1, 1 
blt x1, x2, label_waitloop
addi x3, x3, 1 
and x6, x3, x5 ; masking
sw x6, 0x0(x4)
jalr x0, x0, label_led
nop
nop
nop
nop
;0x007c
bne x5, x8, label_ne_7
ori x5, x0, 1
jalr x0, x0, label_eq_7
:label_ne_7
ori x5, x0, 7
:label_eq_7
lui x9, 0xc0010 ; global interrupt clear address
addi x9, x9, 0xa04 ;
sw x0, 0x0(x9)
mret
nop
nop
nop
nop
