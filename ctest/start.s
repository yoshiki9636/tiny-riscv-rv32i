.option norelax
.globl _start


_start:
        nop ;
        nop ;
        nop ;
        nop ;
	lui sp,0x00020
	lui gp,0x00010
	lui tp,0x0000c
    lui ra,0x00000
    lui s0,0x00010
        j main
        nop ;
        nop ;
        nop ;
