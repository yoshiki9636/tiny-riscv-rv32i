#include <stdio.h>

//#define LP 10
#define LP 1000
#define LP2 200
#define TESTNUM 0x3c00

#include "add_for_cmpl_all.c"
#include "add_for_cmpl2.c"

int main() {
	char cbuf[15] = "Hello World!!\n";

	for (int i = 0; i < 10; i++) {
		printf(cbuf);
		fflush(stdout);
	}

	pass();
	return 0;

}

