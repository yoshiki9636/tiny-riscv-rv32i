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

;addi x4, x0, 0x113 ; bit endian=1, CPHA=1, SPI enable=1
addi x4, x0, 0x111 ; bit endian=1, CPHA=0, SPI enable=1
sw x4, 0x4(x3) ; set value to spi mode reg

addi x4, x0, 0x010 ; sck divider = 16
sw x4, 0x8(x3) ; set value to spi sck divider reg

addi x4, x0, 0x002 ; command length = 2
;addi x4, x0, 0x001 ; command length = 2
sw x4, 0xc(x3) ; set value to command length reg

addi x4, x0, 0x0a5 ; command data byte 2
slli x4, x4, 8 ; shift left 8 bit
addi x4, x4, 0x0c3 ; command data byte 1
sw x4, 0x10(x3) ; set value to command length reg

addi x4, x0, 0x010 ; data length = 16
;addi x4, x0, 0x05 ; data length = 16
sw x4, 0x14(x3) ; set value to command length reg

addi x3, x3, 0x20 ; spi data value address

lui x4, 0xf0f0f ; data0 high
addi x4, x4, 0x0f0 ; data0 low
sw x4, 0x0(x3) ; set value to command data0 reg

lui x4, 0xc3c3c ; data1 high
addi x4, x4, 0x3c3 ; data1 low
sw x4, 0x4(x3) ; set value to command data1 reg

lui x4, 0x00ff0 ; data3 high
addi x4, x4, 0x0ff ; data3 low
sw x4, 0x8(x3) ; set value to command data2 reg

lui x4, 0xa5a5a ; data2 high
addi x4, x4, 0x5a5 ; data2 low
sw x4, 0xc(x3) ; set value to command data3 reg

lui x3, 0xc000f ; spi base address
addi x3, x3, 0x200 ;

addi x4, x0, 0x3 ; SPI run = 1, read/wirte = 1 (write)
sw x4, 0x0(x3) ; set value to spi execution reg

; test finished
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
