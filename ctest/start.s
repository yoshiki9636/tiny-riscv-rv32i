.option norelax
.globl _start


_start:
        nop ;
        nop ;
        nop ;
        nop ;
	lui sp,0x08000
	lui gp,0x00800
	lui tp,0x00600
    lui ra,0x00000
    lui s0,0x00800
        j main
        nop ;
        nop ;
        nop ;
