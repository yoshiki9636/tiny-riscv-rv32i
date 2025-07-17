#include <stdio.h>
//#include <stdlib.h>
#include <math.h>
//#include <string.h>

//#define LP 10
#define LP 1000
#define LP2 200
#define SIZE 4

void uprint( char* buf, int length, int ret );
// workaround for libm_nano.a
int __errno;

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
int _close(void) { return 0; }
int _lseek(void) { return 0; }
int _read(void) { return 0; }
//void _write(void) {}
//void _sbrk_r(void) {}
int abort(void) { return 0; }
void _kill_r(void) { return;}
int _getpid_r(void) { return -1; }
int _fstat_r(void) { return -1; }
int _isatty_r(void) { return -1; }
int _isatty(void) { return -1; }

void pass();
void wait();
double det_cal( double* mat, int s);
int part_mat2( double* mat, double* pmat, int s, int x, int y);
int mat_cofactor( double* mat, int s );
int mat_trans( double* mat, int size);
int matmul_scala( double* mat, double a);
int mat_mul( double* mat1, double* mat2, double* result, int x, int y, int z);
int part_mat( double* mat, double* pmat, int s, int p);
int matrix_print( double* mat, int x, int y);
int double_print( char* cbuf, double value, int digit );

int main() {
	char cbuf[64];
	double mat1[SIZE*SIZE]
		= { 2.0, 0.0, 1.0, 3.0,
		    1.0, 2.0, 2.0, 1.0,
		    1.0, 4.0, 1.0, 3.0,
		    4.0, 0.0, 1.0, 2.0};

	//uprint( "mat1\n", 6, 0 );
	printf( "mat1\n");
	matrix_print( mat1, SIZE, SIZE);

	double det = det_cal( mat1, SIZE);

	//uprint( "det = ", 6, 0 );
	//int length = double_print( cbuf, det, 9 );
	//int length = sprintf(cbuf, "det = %e\n",  det);
	//uprint( cbuf, length, 2 );
	printf("det = %e\n",  det);

	if (det == 0.0) {
		uprint( "no inverse matrix\n", 19, 0 );
	}
	else {
		mat_cofactor(mat1, SIZE);
		mat_trans(mat1, SIZE);
		matmul_scala(mat1, 1.0/det);

		uprint( "inverse matrix\n", 16, 0 );
		matrix_print( mat1, SIZE, SIZE);
	}
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
			//matrix_print( pmat, s-1, s-1);
			//int length = double_print( cbuf, mat[i], 9 );
			//uprint( cbuf, length, 2 );
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

int part_mat2( double* mat, double* pmat, int s, int x, int y) {
	int l = 0;
	for (int j = 0; j < s; j++) {
		int k = 0;
		for (int i = 0; i < s; i++) {
			if (( i != x )&&( j != y )) {
				pmat[l*(s-1)+k] = mat[j*s+i];
			}
			if ( i != x ) {
				k++;
			}
		}
		if ( j != y ) {
			l++;
		}
	}
	return 0;
}
int mat_cofactor( double* mat, int s ) {
	double buf[s*s];
	double pmat[(s-1)*(s-1)];
	for (int j = 0; j < s; j++) {
		for (int i = 0; i < s; i++) {
			double sign = pow( -1.0, i+j+2);
			part_mat2( mat, pmat, s, i, j );
			buf[j*s+i] = sign * det_cal( pmat, s-1);
		}
	}
	for (int i = 0; i < s*s; i++) {
		mat[i] = buf[i];
	}
	return 0;
}
			
int mat_trans( double* mat, int size) {
	double buf[size*size];
	for (int j = 0; j < size; j++) {
		for (int i = 0; i < size; i++) {
			buf[j*size+i] = mat[i*size+j];
		}
	}
	// copy to original array
	for (int i = 0; i < size*size; i++) {
		mat[i] = buf[i];
	}
	return 0;
}

int matmul_scala( double* mat, double a) {
	for (int i = 0; i < SIZE*SIZE; i++) {
		mat[i] = mat[i] * a;
	}
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
	char cbuf2[64];
	for(int j = 0; j < y; j++) {
		for(int i = 0; i < x; i++) {
			//int length = double_print( cbuf2, mat[j*x+i], 9 );
			//int length = sprintf(cbuf2, " %e",  mat[j*x+i]);
			if ( i == x - 1 ) {
				//uprint( cbuf2, length, 2 );
				printf(" %e\n",  mat[j*x+i]);
			}
			else {
				//uprint( cbuf2, length, 1 );
				printf(" %e",  mat[j*x+i]);
			}
		}
	}
	return 0;
}

int double_print( char* cbuf, double value, int digit ) {
	// type 0 : digit  1:hex
	unsigned char buf[64];

	int cntr = 0;
	
	if (value < 0) {
		buf[cntr++] = 0xfe; // for minus
		value = -value;
	}
	double mug = 1.0;
	while(value >= mug) {
		mug *= 10.0;
	}
	if (mug == 1.0) {
		buf[cntr++] = 0; // first zero
		buf[cntr++] = 0xff; // for preiod
	}
	mug /= 10.0;
	for(int i = 0; i < digit; i++) {	
		unsigned char a =(unsigned char)(value / mug);
		buf[cntr++] = a;
		value = value - (double)a * mug;
		if (mug == 1.0) {
			buf[cntr++] = 0xff; // for preiod
		}
		mug /= 10.0;
	}
	if (mug >= 1.0) {
		while(mug >= 1.0) {
			unsigned char a =(unsigned char)(value / mug);
			buf[cntr++] = a;
			value = value - (double)a * mug;
			mug /= 10.0;
			if (cntr >= 64) {
				break;
			}
		}
	}
	for(int i = 0; i < cntr; i++) {	
		if (buf[i] == 0xff) {
			cbuf[i] = 0x2e;
		}
		else if (buf[i] == 0xfe) {
			cbuf[i] = 0x2d;
		}
		else {
			cbuf[i] = buf[i] + 0x30;
		}
	}	
	return cntr;	
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






