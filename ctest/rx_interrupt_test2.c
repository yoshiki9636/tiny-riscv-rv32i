#include <stdio.h>

//#define LP 10
#define LP 1000
#define LP2 200
#define TESTNUM 0x3c00

#include "add_for_cmpl_all.c"
#include "add_for_cmpl2.c"

void __attribute__((interrupt)) interrupt_h();

int main() {
	void (*p_func)();
	register int mask __asm__("x21");
    unsigned int* led = (unsigned int*)0xc000fe00;
    unsigned int* int_enable = (unsigned int*)0xc000fa00;
    unsigned int* int_clr = (unsigned int*)0xc000fa04;
    unsigned int* rx_char = (unsigned int*)0xc000fc0c;
    unsigned int* rx_echoback = (unsigned int*)0xc000fc10;

	*led = 7;
	*rx_echoback = 0; // enable rx echoback
	//*rx_echoback = 1; //disable rx echoback

	// clear char buffer
	*rx_char = 0;
	// for rx interrupt enable
	*int_clr = 0;
	*int_enable = 1;

	p_func = interrupt_h;
	__asm__ volatile("csrw mtvec, %0" : "=r"(p_func));
	unsigned int value = 0x800;
	__asm__ volatile("csrw mie, %0" : "=r"(value));
	// mstatus
	value = 0x8;
	__asm__ volatile("csrw mstatus, %0" : "=r"(value));

	uprint( "start\n", 7, 0);
	*led = 6;
	masked_pass();
	return 0;

}

void __attribute__((interrupt)) interrupt_h() {
    unsigned int* int_clr = (unsigned int*)0xc000fa04;
    unsigned int* rx_char = (unsigned int*)0xc000fc0c;
    unsigned int* tx_char = (unsigned int*)0xc000fc00;
	//register int mask __asm__("x21");
	static int flg;
	flg = (flg == 0) ? 1 : 0;
	mask = (flg == 0) ? 0x7777 : 0x1111;
	//uprint( "pushed\n", 8, 0);

	// reead char
	int char_reg;
	// for normal case
	char_reg = *rx_char;
	// for overread error case : reead twice
	//char_reg = *rx_char;
	// for overwrite error case
	//char_reg = (flg == 0) ? *rx_char : 0; 

	if ((char_reg & 0x300) == 0x300) {
		uprint( "error!! double write and 1st read\n", 21, 0);
		printf("%x\n",char_reg);
	}
	else if ((char_reg & 0x100) == 0) {
		uprint( "error!! double read\n", 21, 0);
		printf("%x\n",char_reg);
	}
	else if ((char_reg & 0x200) != 0) {
		uprint( "error!! double write\n", 22, 0);
		printf("%x\n",char_reg);
	}
	else {
		printf("%x\n",char_reg);
	}

	// interrupt clear
	*int_clr = 0;
}

