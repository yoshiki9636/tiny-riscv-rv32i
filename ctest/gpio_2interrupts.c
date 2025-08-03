#include <stdio.h>
//#include <stdlib.h>
#include <math.h>
//#include <string.h>

//#define LP 10
#define LP 1000
#define LP2 200
#define SIZE 5
// workaround for libm_nano.a
int __errno;
void pass();
void wait();
double det_cal( double* mat, int s);
int part_mat( double* mat, double* pmat, int s, int p);
int matrix_print( double* mat, int x, int y);
int double_print( char* cbuf, double value, int digit );
int int_print( char* cbuf, int value, int type );
void uprint( char* buf, int length, int ret );
void interrupt();

int main() {

	void (*p_func)();
	//register int mask __asm__("x21");
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

	// start external interrupt
	*int_enable = 2;

	*led = 6;

	char cbuf[64];
	unsigned int frc_stat = *frc_ctrl;
	int len = int_print(cbuf, frc_stat, 1);
	uprint( cbuf, len, 2);

	p_func = interrupt;
	__asm__ volatile("csrw mtvec, %0" : "=r"(p_func));
	// enable MEIE MTIE
	unsigned int value = 0x880;
	__asm__ volatile("csrw mie, %0" : "=r"(value));
	// mstatus
	value = 0x8;
	__asm__ volatile("csrw mstatus, %0" : "=r"(value));


	uprint( "start\n", 7, 0);
	*led = 5;

	//char cbuf[64];
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

	*led = 0;
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

	//__asm__ volatile("addi    sp,sp,-128");

	__asm__ volatile("sw  ra,124(zero)");

	__asm__ volatile("sw  t0,120(zero)");
	__asm__ volatile("sw  t1,116(zero)");
	__asm__ volatile("sw  t2,112(zero)");
	__asm__ volatile("sw  t3,108(zero)");
	__asm__ volatile("sw  t4,104(zero)");
	__asm__ volatile("sw  t5,100(zero)");
	__asm__ volatile("sw  t6,96(zero)");

	__asm__ volatile("sw  a0,92(zero)");
	__asm__ volatile("sw  a1,88(zero)");
	__asm__ volatile("sw  a2,84(zero)");
	__asm__ volatile("sw  a3,80(zero)");
	__asm__ volatile("sw  a4,76(zero)");
	__asm__ volatile("sw  a5,72(zero)");
	__asm__ volatile("sw  a6,68(zero)");
	__asm__ volatile("sw  a7,64(zero)");

	__asm__ volatile("sw  s0,60(zero)");
	__asm__ volatile("sw  s1,56(zero)");
	__asm__ volatile("sw  s2,52(zero)");
	__asm__ volatile("sw  s3,48(zero)");
	__asm__ volatile("sw  s4,44(zero)");
	__asm__ volatile("sw  s5,40(zero)");
	__asm__ volatile("sw  s6,36(zero)");
	__asm__ volatile("sw  s7,32(zero)");
	__asm__ volatile("sw  s8,28(zero)");
	__asm__ volatile("sw  s9,24(zero)");
	__asm__ volatile("sw  s10,20(zero)");
	__asm__ volatile("sw  s11,16(zero)");
	__asm__ volatile("sw  tp,12(zero)");
	__asm__ volatile("sw  gp,8(zero)");

	//__asm__ volatile("addi    sp,sp,128");
	__asm__ volatile("addi    s0,sp,128");

	char cbuf[64];
    unsigned int* int_status  = (unsigned int*)0xc000fa04;
    unsigned int* gpio_value  = (unsigned int*)0xc000fe10;
    unsigned int* gpio_input  = (unsigned int*)0xc000fe14;
    unsigned int* gpio_enable = (unsigned int*)0xc000fe18;
    //unsigned int* frc_low  = (unsigned int*)0xc000f800;
    //unsigned int* frc_high = (unsigned int*)0xc000f804;
    unsigned int* frc_ctrl = (unsigned int*)0xc000f810;
	//register int mask __asm__("x21");
	static int value;
	static int enable;

	unsigned int int_stat = *int_status & 0x2;
	unsigned int frc_stat = *frc_ctrl & 0x4;

	unsigned int mstatus;
	uprint( "mstatus = ", 9, 0);
	__asm__ volatile("csrr %0, mstatus" : "=r"(mstatus));
	int len = int_print(cbuf, mstatus, 1);
	uprint( cbuf, len, 2);

	//uprint( "ringing timer!\n", 16, 0);
	//printf("low  counter = %d\n",*frc_low);
	//printf("high counter = %d\n",*frc_high);
	//*gpio_enable = enable;
	//*gpio_value = value;

	uprint( "int_status = ", 13, 0);
	len = int_print(cbuf, int_stat, 1);
	uprint( cbuf, len, 2);
	if (int_stat == 0x2) {
		//unsigned int val = *int_status;
		//*int_status = val & 0xfffffffd;

		enable = (enable == 0x1) ? 0xf : 0x1;
		*gpio_enable = enable;
	}
	//unsigned int frc_stat = *frc_ctrl & 0x4;
	uprint( "frc_stat = ", 11, 0);
	len = int_print(cbuf, frc_stat, 1);
	uprint( cbuf, len, 2);
	if (frc_stat == 0x4) {
		// clear frc counter bit
		//unsigned int val = *frc_ctrl;
		//*frc_ctrl = (val & 0xfffffff9) | 0x2;
		value++;
		*gpio_value = value;
	}

	unsigned int mcause;
	uprint( "mcause = ", 9, 0);
	__asm__ volatile("csrr %0, mcause" : "=r"(mcause));
	len = int_print(cbuf, mcause, 1);
	uprint( cbuf, len, 2);
	
	unsigned int mepc;
	uprint( "mepc = ", 7, 0);
	__asm__ volatile("csrr %0, mepc" : "=r"(mepc));
	//printf("mepc : %x\n",mepc);
	len = int_print(cbuf, mepc, 1);
	uprint( cbuf, len, 2);

	*int_status = 0x0; // clear interrupt status
	*frc_ctrl = 0x3; // clear interrupt status

	// pop from stack
	//__asm__ volatile("addi    sp,sp,-128");
	__asm__ volatile("lw  ra,124(zero)");

	__asm__ volatile("lw  t0,120(zero)");
	__asm__ volatile("lw  t1,116(zero)");
	__asm__ volatile("lw  t2,112(zero)");
	__asm__ volatile("lw  t3,108(zero)");
	__asm__ volatile("lw  t4,104(zero)");
	__asm__ volatile("lw  t5,100(zero)");
	__asm__ volatile("lw  t6,96(zero)");

	__asm__ volatile("lw  a0,92(zero)");
	__asm__ volatile("lw  a1,88(zero)");
	__asm__ volatile("lw  a2,84(zero)");
	__asm__ volatile("lw  a3,80(zero)");
	__asm__ volatile("lw  a4,76(zero)");
	__asm__ volatile("lw  a5,72(zero)");
	__asm__ volatile("lw  a6,68(zero)");
	__asm__ volatile("lw  a7,64(zero)");

	__asm__ volatile("lw  s0,60(zero)");
	__asm__ volatile("lw  s1,56(zero)");
	__asm__ volatile("lw  s2,52(zero)");
	__asm__ volatile("lw  s3,48(zero)");
	__asm__ volatile("lw  s4,44(zero)");
	__asm__ volatile("lw  s5,40(zero)");
	__asm__ volatile("lw  s6,36(zero)");
	__asm__ volatile("lw  s7,32(zero)");
	__asm__ volatile("lw  s8,28(zero)");
	__asm__ volatile("lw  s9,24(zero)");
	__asm__ volatile("lw  s10,20(zero)");
	__asm__ volatile("lw  s11,16(zero)");
	__asm__ volatile("lw  tp,12(zero)");
	__asm__ volatile("lw  gp,8(zero)");

	//__asm__ volatile("addi    sp,sp,128");

	// workaround this should changed by number of parameter
	__asm__ volatile("lw  ra,124(sp)");
	__asm__ volatile("lw  s0,120(sp)");
	__asm__ volatile("addi    sp,sp,128");

	__asm__ volatile("mret");

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

