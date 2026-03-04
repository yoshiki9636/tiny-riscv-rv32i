#include <stdio.h>
#include <math.h>

#define PI 3.14159265358
#define N 128 //データ個数
//#define LP 10
#define LP 1000
#define LP2 200

#include "add_for_cmpl_all.c"

#include "lenna.txt"
 
int main() {
	char cbuf[64];
	int hist[256];

	for(int i = 0; i < 256; i++) {
		hist[i] = 0;
	}

	for(int i = 0; i < 65536; i++) {
		hist[lenna[i]]++;
	}

	uprint( "histogram", 9, 2 );
	for(int i = 0; i < 256; i++) {
		int length = int_print(cbuf, i, 0);
		uprint(cbuf, length, 1);
 		for(int j = 0; j < hist[i] / 16; j++)
			uprint( "*", 1, 0 );
		uprint( "\n", 2, 0 );
 	}

	pass();
	return 0;
} 

