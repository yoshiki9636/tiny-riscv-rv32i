#include <stdio.h>

//#define LP 10
#define LP 1000
#define LP2 200
#define TESTNUM 0x3c00
void uprint( char* buf, int length );
// workaround for using libm_nano.a
int __errno;

char* heap_end = (char*)0x10000;
//void _sbrk_r(void) {}
char* _sbrk(int incr) {
 char* heap_low = (char*)0x10000;
 char* heap_top = (char*)0x18000;
 char *prev_heap_end;

 if (heap_end == 0) {
  heap_end = heap_low;
 }
 prev_heap_end = heap_end;

 if (heap_end + incr > heap_top) {
  /* Heap and stack collision */
  return (char *)0;
 }

 heap_end += incr;
 return (char*) prev_heap_end;
}

int _write(int file, char* ptr, int len)
{
	uprint( ptr, len );
    return len ;
}

// workaround for using libc_nano.a
int _close(void) { return 0; }
int _lseek(void) { return 0; }
int _read(void) { return 0; }
//void _write(void) {}
//void _sbrk_r(void) {}
int abort(void) { return 0; }
void _kill_r(void) { return;}
int _getpid_r(void) { return -1; }
int _fstat_r(void) { return -1; }
int _isatty_r(void) { return -1; }
int _isatty(void) { return -1; }
void inturrpt();
void pass();
void wait();

int mask;

int main() {
	void (*p_func)();
	register int mask __asm__("x21");
    unsigned int* led = (unsigned int*)0xc000fe00;
    unsigned int* int_enable = (unsigned int*)0xc000fa00;
    unsigned int* rx_char = (unsigned int*)0xc000fc0c;

	*led = 7;

	// for rx interrupt enable
	*int_enable = 1;

	p_func = inturrpt;
	__asm__ volatile("csrw mtvec, %0" : "=r"(p_func));
	unsigned int value = 0x800;
	__asm__ volatile("csrw mie, %0" : "=r"(value));

	// clear char buffer
	*rx_char = 0;
	uprint( "start\n", 7);
	*led = 6;
	pass();
	return 0;

}

void inturrpt() {
    unsigned int* int_clr = (unsigned int*)0xc000fa04;
    unsigned int* rx_char = (unsigned int*)0xc000fc0c;
    unsigned int* tx_char = (unsigned int*)0xc000fc00;
	//register int mask __asm__("x21");
	static int flg;
	flg = (flg == 0) ? 1 : 0;
	mask = (flg == 0) ? 0x7777 : 0x1111;
	//uprint( "pushed\n", 8);
	// interrupt clear
	*int_clr = 0;

	// reead char
	int char_reg;
	// for normal case
	char_reg = *rx_char;
	// for overwrite case
	//if(flg == 0) { char_reg = *rx_char; }

	if ((char_reg & 0x100) == 0) {
		uprint( "error!! double read\n", 21);
	}
	else if ((char_reg & 0x200) != 0) {
		uprint( "error!! double write\n", 22);
	}
	else {
		printf("%x\n",char_reg);
		// echo back to tx
		//*tx_char = char_reg & 0xff;
	}

	// workaround
	__asm__ volatile("lw  ra,28(sp)");
	__asm__ volatile("lw  s0,24(sp)");
	__asm__ volatile("lw  s5,20(sp)");
	__asm__ volatile("addi    sp,sp,32");
	__asm__ volatile("mret");
}

void uprint( char* buf, int length ) {
    unsigned int* led = (unsigned int*)0xc000fe00;
    unsigned int* uart_out = (unsigned int*)0xc000fc00;
    unsigned int* uart_status = (unsigned int*)0xc000fc04;

	for (int i = 0; i < length + 1; i++) {
		unsigned int flg = 1;
		while(flg == 1) {
			flg = *uart_status;
		}
		*uart_out = (i == length) ? 0 : buf[i];
		*led = i;
	}
	//return 0;
}

void pass() {
    unsigned int* led = (unsigned int*)0xc000fe00;
    unsigned int val;
	//register int mask __asm__("x21");
	mask = 0x7777;
    val = 0;
    while(1) {
		wait();
		val++;
		*led = val & mask;
    }
}

void wait() {
    unsigned int timer,timer2;
    timer = 0;
	timer2 = 0;
    while(timer2 < LP2) {
        while(timer < LP) {
            timer++;
    	}
        timer2++;
	}
}

