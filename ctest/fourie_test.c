#include <stdio.h>
#include <math.h>

#define PI 3.14159265358
#define N 128 //データ個数
//#define LP 10
#define LP 1000
#define LP2 200

#include "add_for_cmpl_all.c"

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

