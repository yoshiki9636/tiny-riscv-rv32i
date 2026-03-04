#include <stdio.h>
//#include <stdlib.h>
#include <math.h>
//#include <string.h>

//#define LP 10
#define LP 1000
#define LP2 200
#define SIZE 6

#include "add_for_cmpl_all.c"

double det_cal( double* mat, int s);
int part_mat( double* mat, double* pmat, int s, int p);
int matrix_print( double* mat, int x, int y);

int main() {
	char cbuf[64];
	double mat1[SIZE*SIZE];
	
	for (int j = 0; j < SIZE; j++) {
		for (int i = 0; i < SIZE; i++) {
			mat1[j*SIZE+i] = sqrt((double)(j*SIZE+i+1));
		}
	}

	double det = det_cal( mat1, SIZE);

	uprint( "mat1\n", 6, 0 );
	matrix_print( mat1, SIZE, SIZE);
	uprint( "det = ", 6, 0 );
	int length = double_print( cbuf, det, 30 );
	uprint( cbuf, length, 2 );
	pass();
	return 0;
}

double det_cal( double* mat, int s) {
	char cbuf[64];
	if (s <= 0) {
		// error
		return -1.0;
	}
	else if (s == 1) {
		return *mat;
	}
	else if (s == 2) {
		return mat[0] * mat[3] - mat[1] * mat[2];
	}
	else {
		double det = 0.0;
		double sign = 1.0;
		double pmat[(s-1)*(s-1)];
		for (int i = 0; i < s; i++) {
			part_mat( mat, pmat, s, i );
			matrix_print( pmat, s-1, s-1);
			int length = double_print( cbuf, mat[i], 9 );
			uprint( cbuf, length, 2 );
			det = det + sign * mat[i] * det_cal( pmat, s - 1 );
			sign = (sign == 1.0) ? -1.0 : 1.0;
		}
		return det;
	}
}

int part_mat( double* mat, double* pmat, int s, int p) {
	for (int j = 1; j < s; j++) {
		int k = 0;
		for (int i = 0; i < s; i++) {
			if ( i != p ) {
				pmat[(j-1)*(s-1)+k] = mat[j*s+i];
				k++;
			}
		}
	}
	return 0;
}

int matrix_print( double* mat, int x, int y) {
	char cbuf2[64];
	for(int j = 0; j < y; j++) {
		for(int i = 0; i < x; i++) {
			int length = double_print( cbuf2, mat[j*x+i], 9 );
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

