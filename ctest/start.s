.option norelax
.globl _start


_start:
        nop ;
        nop ;
        nop ;
        nop ;
	lui sp,0x00020
	lui gp,0x00010
        j main
        nop ;
        nop ;
        nop ;
