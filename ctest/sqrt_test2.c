#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

//#define LP 10
#define LP 1000
#define LP2 200

#include "add_for_cmpl_all.c"
#include "add_for_cmpl2.c"

int main() {
	char cbuf2[32];

	for (int i = 1; i < 20; i++) {
		double b = sqrt((double)i);
 		//int length = sprintf(cbuf2, "vaule = %e",  b);
		//uprint( cbuf2, length );
 		printf("vaule = %e\n",  b);
	}
	pass();
	return 0;
}

