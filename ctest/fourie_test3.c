#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#define PI 3.14159265358
#define N 128 //データ個数
//#define LP 10
#define LP 1000
#define LP2 200

#include "add_for_cmpl_all.c"
#include "add_for_cmpl2.c"


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
 		x[n].r=0.5+sin(0.1*n)+0.3*sin(0.3*n)+0.15*sin(0.5*n);
 		//x[n].r=0.5+sin(0.1*n)+0.3*sin(0.3*n);
 		x[n].i=0.0;
 	}
 	for(n=0; n<128; n++){
 		//length = sprintf(cbuf, "sin[%d]=%3.3lf\n",n,x[n].r);
		//uprint( cbuf, length, 0 );
 		printf("sin[%d]=%3.3lf\n",n,x[n].r);
		fflush(stdout);
 	}
	uprint( "\n", 2, 0 );

 	for(n=0; n<128; n++){
 		for(k=0; k<30+x[n].r*25; k++)
			uprint( "*", 1, 0 );
		uprint( "\n", 2, 0 );
 	}
 	max=0.0;
 	//length = sprintf(cbuf, "Fourie transform output\n");
	//uprint( cbuf, length, 0 );
 	printf("Fourie transform output\n");
 	for(n=0; n<N; n++){
 		X.r=0;
 		X.i=0;
 		for(k=0; k<N; k++){
 			X.r += x[k].r*cos(2.0*PI*n*k/N) + x[k].i*sin(2.0*PI*n*k/N);
 			X.i += x[k].i*cos(2.0*PI*n*k/N) - x[k].r*sin(2.0*PI*n*k/N);
 		}
 		R[n] = sqrt(X.r*X.r + X.i*X.i);
 		if(R[n]>max) max=R[n];
 		//length = sprintf(cbuf, "X.r=%f, X.i=%f, R(%d)=%f\n", X.r, X.i,n,R[n]);
		//uprint( cbuf, length, 0 );
 		printf("X.r=%lf, X.i=%lf, R(%d)=%lf\n", X.r, X.i,n,R[n]);
		fflush(stdout);
 	}
	uprint( "\n", 2, 0 );
 	//length = sprintf(cbuf, "Fourie transform graph\n");
	//uprint( cbuf, length, 2 );
 	printf("Fourie transform graph\n");
 	for(n=0; n<N; n++){
 		for( k=0; k<R[n]*60/max; k++)
			uprint( "*", 1, 0 );
		uprint( "\n", 2, 0 );
 	}
	pass();
	return 0;
} 

