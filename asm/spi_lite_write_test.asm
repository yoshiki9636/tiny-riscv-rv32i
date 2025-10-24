;*
;* My RISC-V RV32I CPU
;*   Test Code ALU Instructions : No.3
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
; test add

lui x3, 0xc000f ; spi base address
addi x3, x3, 0x200 ;

;addi x4, x0, 0x0d ; bit endian=1, CPOL=1 CPHA=0, SPI enable=1
addi x4, x0, 0x09 ; bit endian=1, CPOL=0 CPHA=0, SPI enable=1
sw x4, 0x0(x3) ; set value to spi mode reg

addi x4, x0, 0x100 ; sck divider = 0x100
sw x4, 0x4(x3) ; set value to spi sck divider reg

addi x5, x0, 0xa5 ; write data
sw x5, 0x8(x3) ; set value to spi mosi fifo

addi x5, x0, 0xc3 ; write data
sw x5, 0x8(x3) ; set value to spi mosi fifo

addi x5, x0, 0xf0 ; write data
sw x5, 0x8(x3) ; set value to spi mosi fifo

addi x5, x0, 0x87 ; write data
sw x5, 0x8(x3) ; set value to spi mosi fifo

addi x5, x0, 0x96 ; write data
sw x5, 0x8(x3) ; set value to spi mosi fifo

addi x5, x0, 0x5a ; write data
sw x5, 0x8(x3) ; set value to spi mosi fifo

addi x5, x0, 0x3c ; write data
sw x5, 0x8(x3) ; set value to spi mosi fifo

addi x5, x0, 0x0f ; write data
sw x5, 0x8(x3) ; set value to spi mosi fifo

:label_wait_finish
lw x5, 0x8(x3) ; get value from spi mosi fifo status
addi x6, x0, 0x200 ; write data
bne x5, x6, label_wait_finish

:label_test_fail_loop
lw x4, 0xC(x3) ; get value form spi miso fifo
andi x4, x4, 0xff ; mask full, empty
addi x5, x0, 0xa5 ; write data
bne x4, x5, label_test_fail_loop

lw x4, 0xC(x3) ; get value form spi miso fifo
andi x4, x4, 0xff ; mask full, empty
addi x5, x0, 0xc3 ; write data
bne x4, x5, label_test_fail_loop

lw x4, 0xC(x3) ; get value form spi miso fifo
andi x4, x4, 0xff ; mask full, empty
addi x5, x0, 0xf0 ; write data
bne x4, x5, label_test_fail_loop

lw x4, 0xC(x3) ; get value form spi miso fifo
andi x4, x4, 0xff ; mask full, empty
addi x5, x0, 0x87 ; write data
bne x4, x5, label_test_fail_loop

lw x4, 0xC(x3) ; get value form spi miso fifo
andi x4, x4, 0xff ; mask full, empty
addi x5, x0, 0x96 ; write data
bne x4, x5, label_test_fail_loop

lw x4, 0xC(x3) ; get value form spi miso fifo
andi x4, x4, 0xff ; mask full, empty
addi x5, x0, 0x5a ; write data
bne x4, x5, label_test_fail_loop

lw x4, 0xC(x3) ; get value form spi miso fifo
andi x4, x4, 0xff ; mask full, empty
addi x5, x0, 0x3c ; write data
bne x4, x5, label_test_fail_loop

lw x4, 0xC(x3) ; get value form spi miso fifo
andi x4, x4, 0xff ; mask full, empty
addi x5, x0, 0x0f ; write data
bne x4, x5, label_test_fail_loop

; test finished
addi x4, x0, 0x00 ; bit endian=0, endian=0 CPHA=0, SPI enable=0
sw x4, 0x0(x3) ; set value to spi mode reg

nop
nop
;lui x2, 10 ; loop max
ori x2, x0, 10 ; loop max
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
