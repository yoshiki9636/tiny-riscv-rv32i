.option norelax
.globl _start


_start:
        nop ;
        nop ;
        nop ;
        nop ;
	la sp, _stack_end
	la gp, __global_pointer$
    lui ra,0x00000
	la s0, _stack_end

	la a0, _bss_start
	la a1, _bss_end
_loop_bss:
	sw zero, (a0)
	addi a0, a0, 4
	bltu a0, a1,  _loop_bss

        j main
        nop ;
        nop ;
        nop ;
