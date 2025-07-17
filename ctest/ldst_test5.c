#include <stdio.h>

//#define LP 10
#define LP 10
#define LP2 20
#define TESTNUM 0x10
void pass();
void fail(unsigned int val1, unsigned int val2, unsigned int val3);
void wait();

unsigned int bufa[TESTNUM];
unsigned int bufb[TESTNUM];

int main() {
	unsigned int* led = (unsigned int*)0xc000fe00;
	*led = 0x7;
	for(unsigned int i = 0; i < TESTNUM; i++) {
		bufa[i] = i;
	}
	*led = 0x6;
	for(unsigned int i = 0; i < TESTNUM; i++) {
		//bufb[i] = (unsigned int)(TESTNUM - 1 - i);
		bufb[TESTNUM-1-i] = i;
		//bufb[i] = i;
	}
	*led = 0x5;
	for(unsigned int i = 0; i < TESTNUM; i++) {
		*led = i;
		if (bufa[i] != bufb[TESTNUM-1-i]) {
			fail(i,(unsigned int)bufa[i],(unsigned int)bufb[TESTNUM-1-i]);
		}
	}
	pass();
	return 0;

}

void pass() {
    unsigned int* led = (unsigned int*)0xc000fe00;
    unsigned int val;
    unsigned int timer,timer2;
    val = 0;
    while(1) {
		wait();
		val++;
		*led = val & 0x7777;
    }
}

void fail(unsigned int val1, unsigned int val2, unsigned int val3) {
    unsigned int* led = (unsigned int*)0xc000fe00;
    unsigned int val;
    unsigned int timer,timer2;
    val = 0;
    unsigned int sw = 0;
    while(1) {
		*led = 0x0;
		wait();
		*led =val1 & 0x7777;
		wait();
		*led = 0x0;
		wait();
		*led = (val2 & 0x77777777) >> 16;
		wait();
		*led = 0x0;
		wait();
		*led = val3 & 0x7777;
		//*led = (val3 & 0x77777777) >> 16;
		wait();
		*led = 0x0;
		wait();
		*led = 0x7777;
		wait();

		val++;
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

