#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define LP 10
#define LP 1000000
#define LP2 200

#include "add_for_cmpl_all.c"
#include "add_for_cmpl2.c"

int main() {
    unsigned int* led = (unsigned int*)0xc000fe00;
	char cbuf[32];
	char cbuf2[32];
	int a = 10000;
	int c = 8400;
	int b,d,e,g;
	int f[8401];
 
	for (b = 0; b < c; b++) {
		f[b] = a / 5;
	}
	e = 0;
	for (c = 8400; c > 0; c -= 14) {
		d = 0;
		for (b = c - 1; b > 0; b--) {
			g = 2 * b - 1;
			d = d * b + f[b] * a;
			f[b] = d % g;
			d /= g;
		}
    	//printf("%04d", e + d / a);
		int length = sprintf(cbuf2, "%04d", e + d / a);
		uprint( cbuf2, length, 0 );
    	e = d % a;
		*led = (unsigned int)c;
	}
	pass();
	return 0;
}

