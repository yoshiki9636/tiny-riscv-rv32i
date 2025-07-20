#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include <string.h>

//#define LP 10
#define LP 1000
#define LP2 200
#define SIZE 1000

void uprint( char* buf, int length, int ret );
// workaround for libm_nano.a
int __errno;

char* heap_end = (char*)0x18000;
//void _sbrk_r(void) {}
char* _sbrk(int incr) {
 char* heap_low = (char*)0x18000;
 char* heap_top = (char*)0x1c000;
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
int _close(void) { return 0; }
int _lseek(void) { return 0; }
int _read(void) { return 0; }
//void _write(void) {}
//void _sbrk_r(void) {}
void abort(void) {}
void _kill_r(void) { return;}
int _getpid_r(void) { return -1; }
int _fstat_r(void) { return -1; }
int _isatty_r(void) { return -1; }
int _isatty(void) { return -1; }

void pass();
void wait();
int swap_val( int* a, int* b);
int part_val(int* arr, int low, int high);
int quick_sort(int* arr, int low, int high);
static void clearbss(void);

int main() {
    unsigned int* led = (unsigned int*)0xc000fe00;
	//char cbuf[64];
	int val[SIZE];
	
	for (int i = 0; i < SIZE; i++) {
		val[i] = rand();
		*led = i & 0x7777;
	}

	quick_sort(val, 0, SIZE-1);

	printf("quick sort : sorted array :\n");
	for (int i = 0; i < SIZE; i++) {
		printf("\nval[%d] = %d\n", i, val[i]);
		*led = i & 0x7777;
	}

	pass();
	return 0;
}


int swap_val( int* a, int* b) {
	int tmp = *a;
	*a = *b;
	*b = tmp;
	return 0;
}

int part_val(int* arr, int low, int high) {
	int pivot = arr[high];
	int min = (low - 1);

	for(int i = low; i < high; i++) {
		if (arr[i] <= pivot) {
			min++;
			swap_val(&arr[min], &arr[i]);
		}
	}
	swap_val(&arr[min+1], &arr[high]);
	return min+1;
}

int quick_sort(int* arr, int low, int high) {
	if (low < high) {
		int pivot = part_val(arr, low, high);
		quick_sort(arr, low, pivot-1);
		quick_sort(arr, pivot+1, high);
	}
}

void uprint( char* buf, int length, int ret ) {
    unsigned int* led = (unsigned int*)0xc000fe00;
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
		*led = i;
	}
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

