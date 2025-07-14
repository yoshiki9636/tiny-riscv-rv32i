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

;loop valuable
lui x5, 01000 ; loop max
;addi x5, x0, 0x10

and x1, x0, x1 ; loop counter
:label_waitloop1
addi x1, x1, 1 
blt x1, x5, label_waitloop1
addi x1, x0, 6 ; LED value
nop
sw x1, 0x0(x2) ; set LED
jalr x0, x0, label_next1
nop
nop
nop
nop
nop

:label_next1
and x1, x0, x1 ; loop counter
:label_waitloop2
addi x1, x1, 1 
blt x1, x5, label_waitloop2
addi x1, x0, 5 ; LED value
nop
sw x1, 0x0(x2) ; set LED
jalr x0, x0, label_next2
nop
nop
nop
nop
nop

:label_next2
and x1, x0, x1 ; loop counter
:label_waitloop3
addi x1, x1, 1 
blt x1, x5, label_waitloop3
addi x1, x0, 4 ; LED value
nop
sw x1, 0x0(x2) ; set LED
jalr x0, x0, label_next3
nop
nop
nop
nop
nop

:label_next3
and x1, x0, x1 ; loop counter
:label_waitloop4
addi x1, x1, 1 
blt x1, x5, label_waitloop4
addi x1, x0, 3 ; LED value
nop
sw x1, 0x0(x2) ; set LED
jalr x0, x0, label_next4
nop
nop
nop
nop
nop

:label_next4
and x1, x0, x1 ; loop counter
:label_waitloop5
addi x1, x1, 1 
blt x1, x5, label_waitloop5
addi x1, x0, 2 ; LED value
nop
sw x1, 0x0(x2) ; set LED
jalr x0, x0, label_next5
nop
nop
nop
nop
nop

:label_next5
and x1, x0, x1 ; loop counter
:label_waitloop6
addi x1, x1, 1 
blt x1, x5, label_waitloop6
addi x1, x0, 1 ; LED value
nop
sw x1, 0x0(x2) ; set LED
jalr x0, x0, label_final
nop
nop
nop
nop
nop


:label_final
addi x1, x0, 5 ; LED value
jalr x0, x0, label_final
nop
nop
nop
nop
nop

:label_fail
addi x1, x0, 1 ; LED value
sw x1, 0x0(x2) ; set LED
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
