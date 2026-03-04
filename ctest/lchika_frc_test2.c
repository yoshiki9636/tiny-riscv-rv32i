#include <stdio.h>

//#define LP 10
#define LP 1000
#define LP2 200
#define TESTNUM 0x3c00

#include "add_for_cmpl_all.c"
#include "add_for_cmpl2.c"

void interrupt();

int mask;

int main() {
	void (*p_func)();
	register int mask __asm__("x21");
    unsigned int* led = (unsigned int*)0xc000fe00;
    unsigned int* int_enable = (unsigned int*)0xc000fa00;
    unsigned int* frc_cmp_low  = (unsigned int*)0xc000f808;
    unsigned int* frc_cmp_high = (unsigned int*)0xc000f80c;
    unsigned int* frc_ctrl = (unsigned int*)0xc000f810;

	*led = 7;

	// for frc setup
	*frc_cmp_low = 0x2faf080; // 1sec @ 50MHz
	*frc_cmp_high = 0;
	// start frc
	*frc_ctrl = 3;

	p_func = interrupt;
	__asm__ volatile("csrw mtvec, %0" : "=r"(p_func));
	// enable MTIE
	unsigned int value = 0x80;
	__asm__ volatile("csrw mie, %0" : "=r"(value));
	// mstatus
	value = 0x8;
	__asm__ volatile("csrw mstatus, %0" : "=r"(value));

	uprint( "start\n", 7, 0);
	*led = 6;
	while(1) { wait(); }
	return 0;

}

void interrupt() {
    unsigned int* led = (unsigned int*)0xc000fe00;
    unsigned int* frc_low  = (unsigned int*)0xc000f800;
    unsigned int* frc_high = (unsigned int*)0xc000f804;
    unsigned int* frc_ctrl = (unsigned int*)0xc000f810;
	//register int mask __asm__("x21");
	static int value;
	uprint( "ringing timer!\n", 16, 0);

	printf("low  counter = %d\n",*frc_low);
	printf("high counter = %d\n",*frc_high);

	// clear both frc counter & interrupt bit
	*frc_ctrl = 3;

	value++;
	*led = value;
	
	// workaround
	__asm__ volatile("lw  ra,28(sp)");
	__asm__ volatile("lw  s0,24(sp)");
	__asm__ volatile("addi    sp,sp,32");
	__asm__ volatile("mret");
}

