;*
;* My RISC-V RV32I CPU
;*   Test Code Jump Instructions : No.7
;*    RV32I code
;* @auther		Yoshiki Kurokawa <yoshiki.k963@gmail.com>
;* @copylight	2021 Yoshiki Kurokawa
;* @license		https://opensource.org/licenses/MIT     MIT license
;* @version		0.1
;*

nop
nop
nop
; clear LED to black
addi x1, x0, 7 ; LED value
lui x2, 0xc0010 ; LED address
addi x2, x2, 0xe00 ;
sw x1, 0x0(x2) ; set LED

lui x4, 0xc0010 ; uart address
addi x4, x4, 0xc00 ;uart address

addi x6, x0, 0x6 ; loop max
addi x7, x0, 0x0 ; loop cntr
:label_wloop

:label_wait00
lw x5, 0x4(x4) ; get fifo status
bne x5,x0, label_wait00
addi x3, x0, 0x48 ; H
sw x3, 0x0(x4) ; set char to uart
:label_wait01
lw x5, 0x4(x4) ; get fifo status
bne x5,x0, label_wait01
addi x3, x0, 0x65 ; e
sw x3, 0x0(x4) ; set char to uart
:label_wait02
lw x5, 0x4(x4) ; get fifo status
bne x5,x0, label_wait02
addi x3, x0, 0x6c ; l
sw x3, 0x0(x4) ; set char to uart
:label_wait03
lw x5, 0x4(x4) ; get fifo status
bne x5,x0, label_wait03
addi x3, x0, 0x6c ; l
sw x3, 0x0(x4) ; set char to uart
:label_wait04
lw x5, 0x4(x4) ; get fifo status
bne x5,x0, label_wait04
addi x3, x0, 0x6f ; o
sw x3, 0x0(x4) ; set char to uart
:label_wait05
lw x5, 0x4(x4) ; get fifo status
bne x5,x0, label_wait05
addi x3, x0, 0x20 ; SPC
sw x3, 0x0(x4) ; set char to uart

:label_wait06
lw x5, 0x4(x4) ; get fifo status
bne x5,x0, label_wait06
addi x3, x0, 0x57 ; W
sw x3, 0x0(x4) ; set char to uart

:label_wait07
lw x5, 0x4(x4) ; get fifo status
bne x5,x0, label_wait07
addi x3, x0, 0x6f ; o
sw x3, 0x0(x4) ; set char to uart

:label_wait08
lw x5, 0x4(x4) ; get fifo status
bne x5,x0, label_wait08
addi x3, x0, 0x72 ; r
sw x3, 0x0(x4) ; set char to uart

:label_wait09
lw x5, 0x4(x4) ; get fifo status
bne x5,x0, label_wait09
addi x3, x0, 0x6c ; l
sw x3, 0x0(x4) ; set char to uart

:label_wait10
lw x5, 0x4(x4) ; get fifo status
bne x5,x0, label_wait10
addi x3, x0, 0x64 ; d
sw x3, 0x0(x4) ; set char to uart

:label_wait11
lw x5, 0x4(x4) ; get fifo status
bne x5,x0, label_wait11
addi x3, x0, 0x21 ; !
sw x3, 0x0(x4) ; set char to uart

:label_wait12
lw x5, 0x4(x4) ; get fifo status
bne x5,x0, label_wait12
addi x3, x0, 0x21 ; !
sw x3, 0x0(x4) ; set char to uart

:label_wait13
lw x5, 0x4(x4) ; get fifo status
bne x5,x0, label_wait13
addi x3, x0, 0x0d ; !
sw x3, 0x0(x4) ; set char to URL

:label_wait14
lw x5, 0x4(x4) ; get fifo status
bne x5,x0, label_wait14
addi x3, x0, 0x0a ; !
sw x3, 0x0(x4) ; set char to uart

:label_wait15
lw x5, 0x4(x4) ; get fifo status
bne x5,x0, label_wait15
addi x3, x0, 0x00 ; !
sw x3, 0x0(x4) ; set char to uart

addi x7, x7, 0x1 ; loop cntr
blt x7,x6, label_wloop

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
