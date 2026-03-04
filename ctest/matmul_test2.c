#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

//#define LP 10
#define LP 1000
#define LP2 200
#define YSIZE 4
#define XSIZE 4
#define ZSIZE 4

#include "add_for_cmpl_all.c"
#include "add_for_cmpl2.c"

int mat_mul( double* mat1, double* mat2, double* result, int x, int y, int z);
int matrix_print( double* mat, int x, int y);

int main() {
	double mat1[ZSIZE*YSIZE];
	double mat2[XSIZE*ZSIZE];
	double result[XSIZE*YSIZE];
	
	for (int j = 0; j < YSIZE; j++) {
		for (int i = 0; i < ZSIZE; i++) {
			mat1[j*ZSIZE+i] = sqrt((double)(j*ZSIZE+i+1));
		}
	}
	for (int j = 0; j < ZSIZE; j++) {
		for (int i = 0; i < XSIZE; i++) {
			mat2[j*XSIZE+i] = sqrt((double)(j*XSIZE+i+21));
		}
	}
	for (int j = 0; j < YSIZE; j++) {
		for (int i = 0; i < XSIZE; i++) {
			result[j*XSIZE+i] = 0.0;
		}
	}

	mat_mul( mat1, mat2, result, XSIZE, YSIZE, ZSIZE);

	uprint( "mat1\n", 6, 0 );
	matrix_print( mat1, ZSIZE, YSIZE);
	uprint( "\nmat2\n", 8, 0 );
	matrix_print( mat2, XSIZE, ZSIZE);
	uprint( "\nresult\n", 10, 0 );
	matrix_print( result, XSIZE, YSIZE);
	pass();
	return 0;
}

int mat_mul( double* mat1, double* mat2, double* result, int x, int y, int z) {
	for(int j = 0; j < y; j++) {
		for(int i = 0; i < x; i++) {
			for(int k = 0; k < z; k++) {
				result[j*x+i] += mat1[j*z+k] * mat2[k*x+i];
			}
		}
	}
	return 0;
}

int matrix_print( double* mat, int x, int y) {
	char cbuf2[32];
	//char cbufe[32];
	for(int j = 0; j < y; j++) {
		for(int i = 0; i < x; i++) {
			sprintf(cbuf2, "%e",  mat[j*x+i]);
			int length = strlen(cbuf2);
			//sprintf(cbufe, "l = %d",  length);
			//length = strlen(cbufe);
 
			if ( i == x - 1 ) {
				uprint( cbuf2, length, 2 );
			}
			else {
				uprint( cbuf2, length, 1 );
			}
		}
	}
	return 0;
}

