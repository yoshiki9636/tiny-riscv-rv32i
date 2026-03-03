#include <stdio.h>
//#include <stdlib.h>
#include <math.h>
//#include <string.h>

//#define LP 10
#define LP 1000
#define LP2 200
#define T 8
#define K 4
#define MY_PI 3.141592653589793238462643

#include "add_for_cmpl_all.c"

double weight( double x, double y, double u, double v);
int dct(double* indata, double* outdata);
int idct(double* indata, double* outdata, int using);
int matrix_print( double* mat, int x, int y);
int double_print( char* cbuf, double value, int digit );
int int_print( char* cbuf, int value, int type );
void uprint( char* buf, int length, int ret );
void pass();
void wait();

int main() {
	char cbuf[64];
	double mat1[T*T];
	double mat2[T*T];
	double mat3[T*T];
	
	for (int j = 0; j < T; j++) {
		for (int i = 0; i < T; i++) {
			mat1[j*T+i] = sqrt((double)(j*T+i+1))*30.0;
		}
	}


	uprint( "mat1", 4, 2 );
	matrix_print( mat1, T, T);
	dct(mat1, mat2);
	uprint( "\ndct\n", 8, 0 );
	matrix_print( mat2, T, T);
	idct(mat2, mat3, K);
	uprint( "\nidct\n", 8, 0 );
	matrix_print( mat3, T, T);
	pass();
	return 0;
}

double weight( double x, double y, double u, double v) {
	char cbuf3[64];
	double cu = 1.0;
	double cv = 1.0;
	if (u == 0.0) { cu /= sqrt(2); }
	if (v == 0.0) { cv /= sqrt(2); }
	double theta =  MY_PI / (2 * T);
	double result  = ( 2 * cu * cv / T ) * cos((2*x+1)*u*theta) * cos((2*y+1)*v*theta);
	return result;
}

int dct(double* indata, double* outdata) {
	for (int i = 0; i < T*T; i++) {
		outdata[i] = 0.0;
	}
	for (int v = 0; v < T; v++) {
		for (int u = 0; u < T; u++) {
			for (int y = 0; y < T; y++) {
				for (int x = 0; x < T; x++) {
					outdata[v*T+u] += indata[y*T+x] * weight((double)x,(double)y,(double)u,(double)v);
				}
			}
		}
	}
	return 0;
}

int idct(double* indata, double* outdata, int using) {
	for (int i = 0; i < T*T; i++) {
		outdata[i] = 0.0;
	}
	for (int v = 0; v < T; v++) {
		for (int u = 0; u < T; u++) {
			for (int y = 0; y < using; y++) {
				for (int x = 0; x < using; x++) {
					outdata[v*T+u] += indata[y*T+x] * weight((double)u,(double)v,(double)x,(double)y);
				}
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

