#include <stdio.h>
#include <string.h>

//#define LP 10
#define LP 1000
#define LP2 200

#include "add_for_cmpl_all.c"


int main() {
	char src[] = "This is bare-metal c test program!!!zzzzzzzzzzzzzzzzzzzzzzzzzzz";
	char dest[64]; 

	memset( dest, 0x41, 64);
	uprint( dest, 64, 2 );

	uprint( src, 64, 2 );

	for (int i = 5; i < 25; i = i + 3) {
		memset( dest, 0x41, 64);
		memcpy( &dest[i], src, i);
		uprint( dest, 64, 2 );
	}

	pass();
	return 0;

}

