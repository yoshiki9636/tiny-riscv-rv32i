;*
;* My RISC-V RV32I CPU
;*   Test Code IF/ID Instructions : No.1
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

addi x3, x0, 0 ; count value
addi x4, x0, 2 ; loop counter1
addi x5, x0, 2 ; loop counter2
addi x6, x0, 0x034 ; modify code point
addi x7, x0, 0x0bc ; modify instruction memory 
;addi x7, x0, 0x0ac ; modify instruction memory 
lw x7, 0x0(x7) ; get modifyed instruction
addi x8, x0, 0x02e ; right value
;addi x8, x0, 0x010 ; nop's value

; increment counter
:loop1
addi x3, x3, 0x1 ; will be modified
addi x3, x3, 0x1 ; normal counter
addi x3, x3, 0x1 ; normal counter
addi x3, x3, 0x1 ; normal counter
addi x4, x4, 0xfff ; loop counter -1
bne x4, x0, loop1 ; loop1
sw x7, 0x0(x6)  ; store modified code
lw x9, 0x0(x6) ; for check
bne x7, x9, failed2 ; 
;fence.i
nop
addi x5, x5, 0xfff ; loop counter -1
addi x4, x0, 2 ; loop counter1
bne x5, x0, loop1 ; loop2
beq x3, x8, finished ; fail when faild
:failed
addi x1, x0, 1 ; LED value
sw x1, 0x0(x2) ; set LED
jalr x0, x0, failed
:failed2
addi x1, x0, 2 ; LED value
sw x1, 0x0(x2) ; set LED
jalr x0, x0, failed2

; test finished
nop
nop
:finished
;lui x2, 01000 ; loop max
ori x2, x0, 10 ; small loop for sim
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
:modify
addi x3, x3, 0x10 ; modify code
nop
nop
nop
