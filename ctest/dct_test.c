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
// workaround for libm_nano.a
int __errno;
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

	//unsigned int flg = 1;
	//while(flg == 1) {
		//flg = *uart_status;
	//}
	//*uart_out = 0x41;

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

int int_print( char* cbuf, int value, int type ) {
	// type 0 : digit  1:hex
	unsigned char buf[32];
	int ofs = 0;
	int cntr = 0;
	if (value == 0) {
        cbuf[0] = 0x30;
        ofs = 1;
    }
	else if (type == 0) { // int
		if (value < 0) {
			cbuf[ofs++] = 0x2d;
			value = -value;
		}
		while(value > 0) {
			buf[cntr++] = (unsigned char)(value % 10);
			value = value / 10;
		}
		for(int i = cntr - 1; i >= 0; i--) {	
			cbuf[ofs++] = buf[i] + 0x30;
		}	
	}
	else { //unsinged int
		unsigned int uvalue = (unsigned int)value;
		while(uvalue > 0) {
			buf[cntr++] = (unsigned char)(uvalue % 10);
			uvalue = uvalue / 10;
		}
		for(int i = cntr - 1; i >= 0; i--) {	
			cbuf[ofs++] = buf[i] + 0x30;
		}	
	}
	return ofs;	
}

