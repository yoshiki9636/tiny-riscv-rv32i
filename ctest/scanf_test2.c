#include <stdio.h>

//#define LP 10
#define LP 1000
#define LP2 200
#define TESTNUM 0x3c00

#include "add_for_cmpl_all.c"
#include "add_for_cmpl2.c"

int main() {
	unsigned int* rx_char = (unsigned int*)0xc000fc0c;
	unsigned int* rx_echoback = (unsigned int*)0xc000fc10;
	*rx_char = 0;
	*rx_echoback = 0; // enable echoback
	//*rx_echoback = 1; // disable echoback

	char cbuf[32] = "Hello World!!\n";
	char sbuf[32] = "";
	int num;

	for (int i = 0; i < 2; i++) {
		printf("%s",cbuf);
		scanf("%d", &num);
		while(getchar() != '\r');
		printf("%d\n",num);
	}

	char cbuf2[32] = "Hello RISC-V!!\n";

	for (int i = 0; i < 2; i++) {
		printf("%s",cbuf2);
		scanf("%s", sbuf);
		while(getchar() != '\r');
		printf("%s\n",sbuf);
	}

	pass();
	return 0;

}

