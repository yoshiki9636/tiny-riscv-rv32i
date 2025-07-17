#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define LP 10
#define LP 1000
#define LP2 200
// workaround for using libm_nano.a
int __errno;
// workaround for using libc_nano.a
void _close(void) {}
void _lseek(void) {}
void _read(void) {}
void _write(void) {}
void _sbrk_r(void) {}
void abort(void) {}
void _kill_r(void) {}
void _getpid_r(void) {}
void _fstat_r(void) {}
void _isatty_r(void) {}
void _isatty(void) {}
void wait();
int int_print( char* cbuf, int value, int type );
void uprint( char* buf, int length, int ret );

int main() {
    unsigned int* led = (unsigned int*)0xc000fe00;
	char cbuf[32];
	char cbuf2[32];
	int r;

	while(1) {
		r = rand();
		*led = (unsigned int)r;

		int length = sprintf(cbuf2, "value = %d", r);
		uprint( cbuf2, length, 2 );
		wait();
	}
	return 0;
}

void uprint( char* buf, int length, int ret ) {
	unsigned int* uart_out = (unsigned int*)0xc000fc00;
	unsigned int* uart_status = (unsigned int*)0xc000fc04;

	for (int i = 0; i < length + ret; i++) {
		unsigned int flg = 1;
		while(flg == 1) {
			flg = *uart_status;
		}
		*uart_out = ((i == length+1)&&(ret == 2)) ? 0x0a :
	                ((i == length)&&(ret == 1)) ? 0x20 :
	                ((i == length)&&(ret == 2)) ? 0x0d : buf[i];
	}
	//return 0;
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

