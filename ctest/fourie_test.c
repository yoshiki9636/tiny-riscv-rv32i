#include <stdio.h>
#include <math.h>

#define PI 3.14159265358
#define N 128 //データ個数
//#define LP 10
#define LP 1000
#define LP2 200
// workaround for libm_nano.a
int __errno;
int int_print( char* cbuf, int value, int type );
int double_print( char* cbuf, double value, int digit );
void uprint( char* buf, int length, int ret );
void pass();
void wait();

typedef struct{
	double r;
	double i;
}complex;

int main() {
	char cbuf[32];

	int n,k;
	int length;
	double R[128],max;
 	complex x[128], X;

    unsigned int* led = (unsigned int*)0xc000fe00;
	uprint(  "t", 1, 2 );
 	for(n=0; n<128; n++){
		*led = n & 0x7777;
 		//x[n].r=0.5+sin(0.1*n)+0.3*sin(0.3*n);
 		x[n].r=0.5+sin(0.1*n)+0.3*sin(0.3*n)+0.15*sin(0.5*n);
 		//x[n].r=1.0;
 		x[n].i=0.0;
 	}
 	//x[1].r=0.5+sin(0.1*1)+0.3*sin(0.3*1)+0.15*sin(0.5*1);
 	for(n=0; n<128; n++){
		uprint(  "sin[", 4, 0 );
		length = int_print(cbuf, n, 0);
		uprint( cbuf, length, 0 );
		uprint(  "]=", 2, 0 );
		length = double_print( cbuf, x[n].r, 3 );
		uprint( cbuf, length, 0 );
		uprint( "\n", 2, 0 );
 	}
	uprint( "\n", 2, 0 );

 	for(n=0; n<128; n++){
 		for(k=0; k<30+x[n].r*25; k++)
			uprint( "*", 1, 0 );
		uprint( "\n", 2, 0 );
 	}
 	max=0.0;
	uprint(  "Fourie transform output\n", 26, 0 );
 	for(n=0; n<N; n++){
 		X.r=0;
 		X.i=0;
 		for(k=0; k<N; k++){
 			X.r += x[k].r*cos(2.0*PI*n*k/N) + x[k].i*sin(2.0*PI*n*k/N);
 			X.i += x[k].i*cos(2.0*PI*n*k/N) - x[k].r*sin(2.0*PI*n*k/N);
 		}
 		R[n] = sqrt(X.r*X.r + X.i*X.i);
 		if(R[n]>max) max=R[n];
		uprint(  "X.r=", 4, 0 );
		length = double_print( cbuf, X.r, 8 );
		uprint( cbuf, length, 0 );
		uprint(  ", X.i=", 6, 0 );
		length = double_print( cbuf, X.i, 8 );
		uprint( cbuf, length, 0 );
		uprint(  ", R[", 4, 0 );
		length = int_print(cbuf, n, 0);
		uprint( cbuf, length, 0 );
		uprint(  "]=", 2, 0 );
		length = double_print( cbuf, R[n], 8 );
		uprint( cbuf, length, 0 );
		uprint( "\n", 2, 0 );
 	}
	uprint( "\n", 2, 0 );
	uprint(  "Fourie transform graph\n", 25, 0 );
 	for(n=0; n<N; n++){
 		for( k=0; k<R[n]*60/max; k++)
			uprint( "*", 1, 0 );
		uprint( "\n", 2, 0 );
 	}
	pass();
	return 0;
} 

int int_print( char* cbuf, int value, int type ) {
	// type 0 : digit  1:hex
	unsigned char buf[32];
	int ofs = 0;
	int cntr = 0;
	if (type == 0) { // int
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

int double_print( char* cbuf, double value, int digit ) {
	// type 0 : digit  1:hex
	unsigned char buf[32];

	int cntr = 0;
	
	if (value < 0) {
		buf[cntr++] = 0xfe; // for minus
		value = -value;
	}
	double mug = 1.0;
	while(value >= mug) {
		mug *= 10.0;
	}
	mug /= 10.0;
	if (value < 1.0) {
		buf[cntr++] = 0;
		buf[cntr++] = 0xff; // for preiod
	}
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
    }
    //return 0;
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

