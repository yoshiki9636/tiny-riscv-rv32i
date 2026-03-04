#include <stdio.h>
#include <stdlib.h>

//#define LP 10
#define LP 1000
#define LP2 200

#include "add_for_cmpl_all.c"
#include "add_for_cmpl2.c"


int main() {
    unsigned int* led = (unsigned int*)0xc000fe00;
	char cbuf[32];
	int r;

	while(1) {
		r = rand();
		*led = (unsigned int)r;
		int length = int_print( cbuf, r, 0 );
		uprint( cbuf, length , 2 );
		wait();
	}
	return 0;
}

