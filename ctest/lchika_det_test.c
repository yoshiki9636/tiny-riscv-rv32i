#include <stdio.h>
//#include <stdlib.h>
#include <math.h>
//#include <string.h>

//#define LP 10
#define LP 1000
#define LP2 200
#define SIZE 4
// workaround for libm_nano.a
int __errno;
void pass();
void wait();
double det_cal( double* mat, int s);
int part_mat( double* mat, double* pmat, int s, int p);
int matrix_print( double* mat, int x, int y);
int double_print( char* cbuf, double value, int digit );
void uprint( char* buf, int length, int ret );
void interrupt();

int main() {

	void (*p_func)();
	register int mask __asm__("x21");
    unsigned int* led = (unsigned int*)0xc000fe00;
    unsigned int* int_enable = (unsigned int*)0xc000fa00;
    unsigned int* frc_cmp_low  = (unsigned int*)0xc000f808;
    unsigned int* frc_cmp_high = (unsigned int*)0xc000f80c;
    unsigned int* frc_ctrl = (unsigned int*)0xc000f810;

	*led = 7;

	// for frc setup
	*frc_cmp_low = 0x17D7840; // 0.5sec @ 50MHz
	*frc_cmp_high = 0;
	// start frc
	*frc_ctrl = 3;

	p_func = interrupt;
	__asm__ volatile("csrw mtvec, %0" : "=r"(p_func));
	// enable MTIE
	unsigned int value = 0x80;
	__asm__ volatile("csrw mie, %0" : "=r"(value));

	uprint( "start\n", 7, 0);
	*led = 6;

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

	while(1) { wait(); }
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
		//*led = i;
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

void interrupt() {
    unsigned int* led = (unsigned int*)0xc000fe00;
    unsigned int* frc_low  = (unsigned int*)0xc000f800;
    unsigned int* frc_high = (unsigned int*)0xc000f804;
    unsigned int* frc_ctrl = (unsigned int*)0xc000f810;
	//register int mask __asm__("x21");
	static int value;
	//uprint( "ringing timer!\n", 16, 0);

	//printf("low  counter = %d\n",*frc_low);
	//printf("high counter = %d\n",*frc_high);

	// clear both frc counter & interrupt bit
	*frc_ctrl = 3;

	value++;
	*led = value;
	
	// workaround
	__asm__ volatile("lw  ra,28(sp)");
	__asm__ volatile("lw  s0,24(sp)");
	//__asm__ volatile("lw  s5,20(sp)");
	__asm__ volatile("addi    sp,sp,32");
	__asm__ volatile("mret");
}

