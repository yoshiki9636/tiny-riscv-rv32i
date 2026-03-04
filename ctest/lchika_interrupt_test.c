#include <stdio.h>

//#define LP 10
#define LP 1000
#define LP2 200
#define TESTNUM 0x3c00

#include "add_for_cmpl_all.c"

void interrupt();

int mask;

int main() {
	void (*p_func)();
	register int mask __asm__("x21");
    unsigned int* led = (unsigned int*)0xc000fe00;
    unsigned int* int_enable = (unsigned int*)0xc000fa00;

	*led = 7;

	// for external interrupt enable
	*int_enable = 2;

	p_func = interrupt;
	__asm__ volatile("csrw mtvec, %0" : "=r"(p_func));
	// enable MEIE
	unsigned int value = 0x800;
	__asm__ volatile("csrw mie, %0" : "=r"(value));
	// mstatus
	value = 0x8;
	__asm__ volatile("csrw mstatus, %0" : "=r"(value));

	uprint( "start\n", 7, 0);
	*led = 6;
	pass();
	return 0;

}

void interrupt() {
    unsigned int* int_clr = (unsigned int*)0xc000fa04;
	//register int mask __asm__("x21");
	static int flg;
	flg = (flg == 0) ? 1 : 0;
	mask = (flg == 0) ? 0x7777 : 0x1111;
	uprint( "pushed\n", 8, 0);
	*int_clr = 0;
	// workaround
	__asm__ volatile("lw  ra,28(sp)");
	__asm__ volatile("lw  s0,24(sp)");
	//__asm__ volatile("lw  s5,20(sp)");
	__asm__ volatile("addi    sp,sp,32");
	__asm__ volatile("mret");
}

