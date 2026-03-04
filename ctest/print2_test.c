#include <stdio.h>

//#define LP 10
#define LP 1000
#define LP2 200

#include "add_for_cmpl_all.c"

void fail();

int main() {
	//char cbuf[15] = "Hello World!!\n";
	char cbuf2[32] = "Hello";

	int a = -12345;
	float b = 543.21;

	int length = int_print( cbuf2, a, 0 );
	if (length > 10) {
		fail();
	}
	for (int i = 0 ; i < 6; i++) {
		uprint( cbuf2, length, 2 );
	}

	pass();
	return 0;

}

void fail() {
    unsigned int* led = (unsigned int*)0xc000fe00;
    unsigned int val;
    unsigned int timer,timer2;
    val = 0;
    while(1) {
		wait();
		val++;
		*led = val & 0x1111;
    }
}

