#include <stdio.h>
#include <math.h>
#include <string.h>

//#define LP 10
#define LP 1000
#define LP2 200
#define T 8
#define K 8
#define S 8
#define MY_PI 3.141592653589793238462643

//#include "lenna.txt"
#include "lenna2.txt"
// workaround for libm_nano.a

#include "add_for_cmpl_all.c"
#include "add_for_cmpl2.c"

int print_coodinate(int x, int y, int type);
int uchar2double(unsigned char* indata, double* outdata, int size);
int double2uchar(double* indata, unsigned int* outdata, int size);
int get_tile(unsigned char* indata, unsigned char* outdata, int xsize, int x, int y, int size);
int put_tile(unsigned int* indata, unsigned int* outdata, int xsize, int x, int y, int size);
int get_dctval(double* indata, double* outdata, int xsize, int x, int y, int size);
int put_dctval(double* indata, double* outdata, int xsize, int x, int y, int dummy, int size);
double weight( double x, double y, double u, double v);
//int dct_dummy(double* indata, double* outdata);
int dct(double* indata, double* outdata);
int idct(double* indata, double* outdata, int using);
int matrix_print( double* mat, int x, int y);
//int matrix_print_dummy( double* mat, int x, int y);
int double_print( char* cbuf, double value, int digit );
int int_print( char* cbuf, int value, int type );
void uprint( char* buf, int length, int ret );
//void uprint_dummy( char* buf, int length, int ret );
void pass();
void wait();

int main() {
	char cbuf[64];
	double mat1[T*T];
	double mat2[T*T];
	double mat3[T*T];
	unsigned char cmat1[T*T];
	unsigned int cmat2[T*T];
	double dct_data[S*S];
	unsigned int idct_data[S*S];
	
    unsigned int* led = (unsigned int*)0xc000fe00;

	*led = 0x07;
	// dct
	for (int y = 0; y < S; y = y + T) {
		for (int x = 0; x < S; x = x + T) {
			get_tile( &lenna[0], &cmat1[0], S, x, y, T);
			uchar2double(&cmat1[0], &mat1[0], T*T);
			uprint( "dct mat1 ", 9, 1 );
			uprint( "d", 1, 0 );
			print_coodinate(x, y, 2);
			matrix_print( &mat1[0], T, T);
			*led = 0x01;

			put_dctval(&mat1[0], &dct_data[0], S, x, y, 0, T);
			*led = 0x02;

			dct(mat1, mat2);
			*led = 0x03;

			put_dctval(&mat2[0], &dct_data[0], S, x, y, 0, T);
			*led = 0x04;

			uprint( "dct mat2 ", 9, 1 );
			print_coodinate(x, y, 2);
			matrix_print( &mat2[0], T, T);

			put_dctval(&mat2[0], &dct_data[0], S, x, y, 0, T);
		}
	}
	//matrix_print( dct_data, S, S);

	// idct
	for (int y = 0; y < S; y = y + T) {
		for (int x = 0; x < S; x = x + T) {
			get_dctval( dct_data, mat3, S, x, y, T);
			uprint( "idct mat1 ", 10, 1 );
			print_coodinate(x, y, 2);
			matrix_print( mat3, T, T);

			idct(mat3, mat2, K);

			uprint( "idct mat2 ", 10, 1 );
			print_coodinate(x, y, 2);
			matrix_print( mat2, T, T);
			double2uchar(mat2, cmat2, T*T);
			put_tile(cmat2, idct_data, S, x, y, T);
		}
	}

	uprint( "mat2", 4, 2 );
	//for ( int i = 0; i < S*S; i++) {

	pass();
	return 0;
}


int print_coodinate(int x, int y, int type) {
	char cbuf[64];

	printf("( %d, %d )\n",x,y);
	//uprint( " ( ", 3, 1 );
	//int length = int_print( cbuf, x, 0 );
	//uprint( cbuf, length, 1 );
	//uprint( " , ", 3, 1 );
	//length = int_print( cbuf, y, 0 );
	//uprint( cbuf, length, 1 );
	//uprint( " )", 3, type );
	return 0;
}

int get_dctval(double* indata, double* outdata, int xsize, int x, int y, int size) {
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			outdata[i*size+j] = indata[(y+i)*xsize+x+j];
		}
		//memcpy( &outdata[i*size], &indata[(y+i)*xsize+x], size*sizeof(double));
	}
	return 0;
}

int put_dctval(double* indata, double* outdata, int xsize, int x, int y, int dummy, int size) {
	char cbuf2[64];
    unsigned int* led = (unsigned int*)0xc000fe00;
	//*led = dummy >> 3;
	//*led = dummy;
	int length = int_print( cbuf2, dummy, 0 );
	//uprint_dummy( cbuf2, length, 2 );
	uprint( cbuf2, length, 2 );

	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			outdata[(y+i)*xsize+x+j] = indata[i*size+j];
			//length = double_print( cbuf2, indata[i*size+j], 9 );
			length = sprintf(cbuf2, "%e", indata[i*size+j]);
			if ( j == size - 1 ) {
				uprint( cbuf2, length, 2 );
			}
			else {
				uprint( cbuf2, length, 1 );
			}
		}
		//memcpy( &outdata[(y+i)*xsize+x], &indata[i*size], size*sizeof(double));
	}
	return 0;
}

int get_tile(unsigned char* indata, unsigned char* outdata, int xsize, int x, int y, int size) {
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			outdata[i*size+j] = indata[(y+i)*xsize+x+j];
		}
		//memcpy( &outdata[i*size], &indata[(y+i)*xsize+x], size*sizeof(unsigned char));
	}
	return 0;
}

int put_tile(unsigned int* indata, unsigned int* outdata, int xsize, int x, int y, int size) {
	for (int i = 0; i < size; i++) {
		for (int j = 0; j < size; j++) {
			outdata[(y+i)*xsize+x+j] = indata[i*size+j];
		}
		//memcpy( &outdata[(y+i)*xsize+x], &indata[i*size], size*sizeof(unsigned int));
	}
	return 0;
}

int double2uchar(double* indata, unsigned int* outdata, int size) {
	for (int i = 0; i < size; i++) {
		double tmp = (indata[i] > 255.0) ? 255.0 :
                     (indata[i] < 0.0) ? 0.0 : indata[i];
		outdata[i] = (unsigned int)tmp;
	}
	return 0;
}

int uchar2double(unsigned char* indata, double* outdata, int size) {
	for (int i = 0; i < size; i++) {
		outdata[i] = (double)indata[i];
		printf ("%d -> %e\n",indata[i],(double)indata[i]);
	}
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

//int dct_dummy(double* indata, double* outdata) {
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

//int matrix_print_dummy( double* mat, int x, int y) {
int matrix_print( double* mat, int x, int y) {
	char cbuf2[64];
	for(int j = 0; j < y; j++) {
		for(int i = 0; i < x; i++) {
			//int length = double_print( cbuf2, mat[j*x+i], 9 );
			int length = sprintf(cbuf2, "%e",  mat[j*x+i]);
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

