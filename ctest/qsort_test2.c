#include <stdio.h>
#include <stdlib.h>
#include <math.h>
//#include <string.h>

//#define LP 10
#define LP 1000
#define LP2 200
#define SIZE 1000

#include "add_for_cmpl_all.c"
#include "add_for_cmpl2.c"

int swap_val( int* a, int* b);
int part_val(int* arr, int low, int high);
int quick_sort(int* arr, int low, int high);

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

