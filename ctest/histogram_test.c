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

#include "lenna.txt"
 
int main() {
	char cbuf[64];
	int hist[256];

	for(int i = 0; i < 256; i++) {
		hist[i] = 0;
	}

	for(int i = 0; i < 65536; i++) {
		hist[lenna[i]]++;
	}

	uprint( "histogram", 9, 2 );
	for(int i = 0; i < 256; i++) {
		int length = int_print(cbuf, i, 0);
		uprint(cbuf, length, 1);
 		for(int j = 0; j < hist[i] / 16; j++)
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

