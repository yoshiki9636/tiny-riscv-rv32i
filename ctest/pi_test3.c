#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#define LP 10
#define LP 1000
#define LP2 200
// workaround for using libm_nano.a
int __errno;

void uprint( char* buf, int length, int ret );

char* heap_end = (char*)0x8000;
//void _sbrk_r(void) {}
char* _sbrk(int incr) {
 char* heap_low = (char*)0x8000;
 char* heap_top = (char*)0xc000;
 char *prev_heap_end;

 if (heap_end == 0) {
  heap_end = heap_low;
 }
 prev_heap_end = heap_end;

 if (heap_end + incr > heap_top) {
  /* Heap and stack collision */
  return (char *)0;
 }

 heap_end += incr;
 return (char*) prev_heap_end;
}

int _write(int file, char* ptr, int len)
{
    uprint( ptr, len, 0 );
    return len ;
}
// workaround for using libc_nano.a
void _close(void) {}
void _lseek(void) {}
void _read(void) {}
//void _write(void) {}
//void _sbrk_r(void) {}
void abort(void) {}
void _kill_r(void) {}
void _getpid_r(void) {}
void _fstat_r(void) {}
void _isatty_r(void) {}
void _isatty(void) {}
void pass();
void wait();

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
		*led = (unsigned int)b;
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
    	printf("%04d", e + d / a);
		//unsigned int value =  e + d / a;
		//int length = sprintf(cbuf2, "%04d", value);
		//uprint( cbuf2, length, 0 );
    	e = d % a;
		*led = (unsigned int)c;
	}
	printf("\n");
	pass();
	return 0;
}

void uprint( char* buf, int length, int ret ) {
	unsigned int* uart_out = (unsigned int*)0xc000fc00;
	unsigned int* uart_status = (unsigned int*)0xc000fc04;

	for (int i = 0; i < length + 3; i++) {
		unsigned int flg = 1;
		while(flg == 1) {
			flg = *uart_status;
		}
		*uart_out = (i == length+2) ? 0 :
                    ((i == length+1)&&(ret != 1)) ? 0 :
                    ((i == length+1)&&(ret == 1)) ? 0x0a :
                    ((i == length)&&(ret == 0)) ? 0 :
                    ((i == length)&&(ret == 1)) ? 0x0d :
                    ((i == length)&&(ret == 2)) ? 0x20 : buf[i];
	}
	//return 0;
}

void pass() {
    unsigned int* led = (unsigned int*)0xc000fe00;
    unsigned int val;
    unsigned int timer,timer2;
    val = 0;
    while(1) {
        wait();
        val++;
        *led = val & 0x7777;
    }
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

