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

lui x13, 0xc0010 ; uart address
addi x13, x13, 0xc00 ;uart address


lui x6, 0x00001 ; destiation
lui x7, 0x00030 ;
addi x7, x7, 0x0 ;
lui x8, 0x00000 ; counter

:label_loopa
sb x8, 0x0(x6) ;byte write
addi x6, x6, 1 ;
addi x8, x8, 1 ;
blt x8, x7, label_loopa

addi x1, x1, 0xfff ; LED value
sw x1, 0x0(x2) ; set LED

lui x5, 0x00001 ; source
addi x5, x5, 0x000 ;
lui x6, 0x00030 ; destiation
addi x6, x6, 0xfff ;
lui x7, 0x00000 ;
addi x7, x7, 0x7ff ;
lui x8, 0x00000 ; counter

:label_loop2
lb x9, 0x0(x5) ;byte read
sb x9, 0x0(x6) ;byte write
addi x5, x5, 1 ;
addi x6, x6, 0xfff ;
addi x8, x8, 1 ;
blt x8, x7, label_loop2

addi x1, x1, 0xfff ; LED value
sw x1, 0x0(x2) ; set LED

lui x5, 0x00001 ; source
addi x5, x5, 0x000 ;
lui x6, 0x00030 ; destiation
addi x6, x6, 0xfff ;
lui x7, 0x00000 ;
addi x7, x7, 0x7ff ;
lui x8, 0x00000 ; counter

:label_loop3
lb x9, 0x0(x5) ;byte read
lb x10, 0x0(x6) ;byte write

:label_wait00
lw x14, 0x4(x13) ; get fifo status
bne x14,x0, label_wait00
andi x15, x9, 0xf0 ; upper x9 
srli x15, x15, 4 ; right shift upper x9 
addi x15, x15, 0x30 ; make char
sw x15, 0x0(x13) ; set char to uart

:label_wait01
lw x14, 0x4(x13) ; get fifo status
bne x14,x0, label_wait01
andi x15, x9, 0xf ; lower x9 
addi x15, x15, 0x30 ; make char
sw x15, 0x0(x13) ; set char to uart

:label_wait02
lw x14, 0x4(x13) ; get fifo status
bne x14,x0, label_wait02
andi x15, x10, 0xf0 ; upper x10 
srli x15, x15, 4 ; right shift upper x10
addi x15, x15, 0x30 ; make char
sw x15, 0x0(x13) ; set char to uart

:label_wait03
lw x14, 0x4(x13) ; get fifo status
bne x14,x0, label_wait03
andi x15, x10, 0xf ; lower x10 
addi x15, x15, 0x30 ; make char
sw x15, 0x0(x13) ; set char to uart

:label_wait04
lw x14, 0x4(x13) ; get fifo status
bne x14,x0, label_wait04
addi x15, x0, 0x20 ; make char
sw x15, 0x0(x13) ; set char to uart

addi x5, x5, 1 ;
addi x6, x6, 0xfff ;
bne x9, x10, label_fail
addi x8, x8, 1 ;
blt x8, x7, label_loop3
jalr x0, x0, label_pass

:label_fail
addi x1, x0, 1 ; LED value
sw x1, 0x0(x2) ; set LED
jalr x0, x0, label_fail
nop
nop

:label_pass
;lui x2, 10 ; loop max
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
