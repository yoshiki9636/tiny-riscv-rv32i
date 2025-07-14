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
; clear LED to black
addi x1, x0, 7 ; LED value
lui x2, 0xc0010 ; LED address
addi x2, x2, 0xe00 ;
sw x1, 0x0(x2) ; set LED

lui x5, 0x00000 ; source
lui x6, 0x00002 ; destiation
lui x7, 0x00000 ; 
;
addi x7, x1, 0x7ff ;
lui x8, 0x00000 ; counter

:label_loop1
lw x9, 0x0(x5) ;byte read
sw x9, 0x0(x6) ;byte write
addi x5, x5, 4 ;
addi x6, x6, 4 ;
addi x8, x8, 1 ;
blt x8, x7, label_loop1

nop
nop

lui x2, 01000 ; loop max
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
