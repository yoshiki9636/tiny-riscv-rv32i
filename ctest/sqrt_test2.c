#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

//#define LP 10
#define LP 1000
#define LP2 200

// workaround for libm_nano.a
int __errno;

void uprint( char* buf, int length );

char* heap_end = (char*)0x40000;
//void _sbrk_r(void) {}
char* _sbrk(int incr) {
 char* heap_low = (char*)0x40000;
 char* heap_top = (char*)0x7f000;
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
    uprint( ptr, len );
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

void uprint( char* buf, int length ) {
    unsigned int* led = (unsigned int*)0xc000fe00;
    unsigned int* uart_out = (unsigned int*)0xc000fc00;
    unsigned int* uart_status = (unsigned int*)0xc000fc04;

	for (int i = 0; i < length + 2; i++) {
		unsigned int flg = 1;
		while(flg == 1) {
			flg = *uart_status;
		}
		*uart_out = (i == length+1) ? 0x0a :
		            (i == length) ? 0x0d : buf[i];
		*led = i;
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

