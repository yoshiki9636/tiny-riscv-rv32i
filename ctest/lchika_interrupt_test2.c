#include <stdio.h>

//#define LP 10
#define LP 10000
#define LP2 2000
#define TESTNUM 0x3c00

#include "add_for_cmpl_all.c"
#include "add_for_cmpl2.c"

void __attribute__((interrupt)) interrupt_h();

extern unsigned int mask;

int main() {
	void (*p_func)();
	//register int mask __asm__("x21");
    static volatile unsigned int* led = (unsigned int*)0xc000fe00;
    static volatile unsigned int* int_enable = (unsigned int*)0xc000fa00;
    static volatile unsigned int* int_clr = (unsigned int*)0xc000fa04;

	*led = 7;

	// for external interrupt enable
	*int_clr = 0;
	*int_enable = 2;

	p_func = interrupt_h;
	__asm__ volatile("csrw mtvec, %0" : "=r"(p_func));
	// enable MEIE
	unsigned int value = 0x800;
	__asm__ volatile("csrw mie, %0" : "=r"(value));
	// mstatus
	value = 0x8;
	__asm__ volatile("csrw mstatus, %0" : "=r"(value));

	uprint( "start\n", 7, 0);
	*led = 6;
	//pass();
	masked_pass();

	return 0;

}

static int flg = 0;

void __attribute__((interrupt)) interrupt_h() {
    static volatile unsigned int* int_clr = (unsigned int*)0xc000fa04;
    static volatile unsigned int* led = (unsigned int*)0xc000fe00;
	uprint( "pushed", 6, 2);

	*led = 1;
    // set mstatus
    unsigned int value;
    unsigned int mepc;
    __asm__ volatile("csrr %0, mcause" : "=r"(value));
    __asm__ volatile("csrr %0, mepc" : "=r"(mepc));
    //printf(" %x",value);
    //printf(" %x",mepc);
    //fflush(stdout);


    if (value == 2) {
		uprint( "illegal ops", 11, 2);
        __asm__ volatile("csrr %0, mtval" : "=r"(value));
        printf(" %x",value);
        __asm__ volatile("csrr %0, mtvec" : "=r"(value));
        printf(" %x",value);
        fflush(stdout);
        //mepc += 4;
        //__asm__ volatile("csrw mepc, %0" : "=r"(mepc));
        fail();
    }
    else if (( *int_clr & 0x2) != 0) {
		flg = (flg == 0) ? 1 : 0;
		mask = (flg == 0) ? 0x7777 : 0x1111;
		uprint( "cleard", 7, 2);
		*int_clr = 0;
	}
	else {
		uprint( "noise!", 6, 2);
		*int_clr = 0;
	}
}

