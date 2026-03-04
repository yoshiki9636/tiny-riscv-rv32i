#include <stdio.h>
#include <math.h>

//#define LP 10
#define LP 1000
#define LP2 200

#include "add_for_cmpl_all.c"

int main() {
	char cbuf2[32];

	for (int i = 1; i < 11; i++) {
		double b = sqrt((double)i);
		//double b = sqrt(2.0);
		int length = double_print( cbuf2, b, 9 );

		uprint( cbuf2, length, 2 );
	}
	pass();
	return 0;
}

