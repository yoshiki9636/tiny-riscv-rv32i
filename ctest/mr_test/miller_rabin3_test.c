#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
//#include "/opt/riscv32i/gmp/include/gmp.h"
#include "mini-gmp.h"
#include "mini-gmp.c"

#define LP 10
//#define LP 1000
#define LP2 200

#define BLEN 32
#define ILEN (32/32)

// workaround for libm_nano.a

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

int __errno;

extern char _heap_start;
extern char _heap_end;

char* heap_end = (char*)(&_heap_start);
char* heap_low = (char*)(&_heap_start);
char* heap_top = (char*)(&_heap_end);

//void _sbrk_r(void) {}
char* _sbrk(int incr) {
	char *prev_heap_end;
	char cbuf[64];

	if (heap_end == (char*)0) {
		heap_end = (char*)(&_heap_start);
		heap_low = (char*)(&_heap_start);
		heap_top = (char*)(&_heap_end);
	}
	prev_heap_end = heap_end;

	if (heap_end + incr > heap_top) {
		/* Heap and stack collision */
		uprint( "srbk collision : ", 17, 0 );
		int length = int_print( cbuf, (unsigned int)heap_end, 1 );
		uprint( cbuf, length, 2 );
		return (char *)0;
	}

	heap_end += incr;

	//uprint( "srbk : ", 7, 0 );
	//int length = int_print( cbuf, (unsigned int)heap_end, 1 );
	//uprint( cbuf, length, 2 );
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
void abort(void) { while(1) {} }
void _kill_r(void) { return;}
int _getpid_r(void) { return -1; }
int _fstat_r(void) { return -1; }
int _isatty_r(void) { return -1; }
int _isatty(void) { return -1; }

// -------------------

size_t fwrite(const void *, size_t, size_t, FILE *) { return 0; }
void raise(void) { }
//void* realloc(void*, size_t) { return NULL; }
//char* strchr(const char*, int) { return NULL; }
char* nl_langinfo(void) { return ""; }
//int snprintf(char *__restrict, size_t, const char *__restrict, ...) { return 0; }

// by AI
void* realloc(void *ptr, size_t new_size) {
    if (ptr == NULL) return malloc(new_size);
    if (new_size == 0) { free(ptr); return NULL; }

    void *new_ptr = malloc(new_size);
    if (new_ptr == NULL) return NULL;

    free(ptr);
    return new_ptr;
}

char *strchr(const char *s, int c) {
    char ch = (char)c;

    while (*s != '\0') {
        if (*s == ch) {
            return (char *)s;
        }
        s++;
    }
    if (ch == '\0') {
        return (char *)s;
    }
    return NULL;
}

int snprintf(char *str, size_t size, const char *format, ...)
{
    va_list ap;
    int result;

    va_start(ap, format);
    result = vsnprintf(str, size, format, ap);
    va_end(ap);

    return result;
}

// --------------------

char _pbuf[4096];

// 簡易的なビット乱数生成（mini-gmp用）
void mpz_urandomb_custom(mpz_t rop, size_t bits) {
    mpz_set_ui(rop, 0);
    for (size_t i = 0; i < bits; i += 30) { // rand()は通常15bit以上なので30bitずつ
        mpz_mul_2exp(rop, rop, 30);
        mpz_add_ui(rop, rop, rand() & 0x3FFFFFFF);
    }
    // 指定ビット数でマスク
    mpz_t mask;                
    mpz_init(mask);
    mpz_ui_pow_ui(mask, 2, bits);
    mpz_sub_ui(mask, mask, 1);
    mpz_and(rop, rop, mask);
    mpz_clear(mask);
}

// n以下のランダムなmpz_tを生成する関数
void generate_random_under_n(mpz_t result, const mpz_t n) {
    if (mpz_cmp_ui(n, 0) <= 0) {
        mpz_set_ui(result, 0);
        return;
    }

    size_t bit_count = mpz_sizeinbase(n, 2);
    
    do {
        // bit_countビットの乱数を生成してresultに格納
        mpz_urandomb_custom(result, bit_count); 
        // 生成した乱数がnより大きい場合、もう一度生成
    } while (mpz_cmp(result, n) >= 0);
}


// -------------------
int get_random(unsigned int blen, mpz_t rnd) {

	mpz_init (rnd);
	//mpz_urandomb(rnd, state, blen);
	mpz_urandomb_custom(rnd, blen);
	mpz_setbit(rnd,0);
	mpz_setbit(rnd,blen-1);
	return 0;
}

// r = (a * b) % m
int mul_mod_bnum(mpz_t a, mpz_t b, mpz_t m, mpz_t r ) {
	mpz_t tmp;
	mpz_init (tmp);
	mpz_mul(tmp, a, b);
	mpz_mod(r, tmp, m);
	//unsigned int zz = (unsigned int)mpz_get_ui(r);
	//unsigned int aa = (unsigned int)mpz_get_ui(a);
	//unsigned int bb = (unsigned int)mpz_get_ui(b);
	//unsigned int cc = (unsigned int)mpz_get_ui(m);
	//unsigned int dd = (unsigned int)mpz_get_ui(tmp);
	//mpz_fdiv_q_2exp (tmp, tmp, 32); // rsh
	//unsigned int ee = (unsigned int)mpz_get_ui(tmp);
	//printf("mm %08x %08x %08x %08x %08x %08x\n",aa,bb,cc,ee,dd,zz);
	return 0;
}

// Modular Exponentiation: res = base^exp % mod
int mod_exp_bnum(mpz_t base, mpz_t exp, mpz_t mod, mpz_t res) {
    unsigned int* led = (unsigned int*)0xc000fe00;
	mpz_t tmp;
	mpz_t b;
	mpz_t e;
	mpz_set_ui(res, 1);
	mpz_init_set(b, base);
	mpz_init_set(e, exp);
	mpz_init(tmp);
	int cntr = 0;

	while(mpz_sgn(e) != 0) {
		if (mpz_scan1(e,0) == 0) {
			mul_mod_bnum(res, b, mod, tmp);
			mpz_init_set(res, tmp);
		}
		*led = cntr;
		cntr++;
		mul_mod_bnum(b, b, mod, b);
		mpz_fdiv_q_2exp (e, e, 1); // rsh
	}
	//unsigned int zz = (unsigned int)mpz_get_ui(res);
	//unsigned int aa = (unsigned int)mpz_get_ui(base);
	//unsigned int bb = (unsigned int)mpz_get_ui(exp);
	//unsigned int cc = (unsigned int)mpz_get_ui(mod);
	//printf("me %08x %08x %08x %08x\n",aa,bb,cc,zz);
	return 0;
}


// millar rabin primary test
//int mr_primary_test(int k, gmp_randstate_t state, mpz_t n) {
int mr_primary_test(int k, mpz_t n) {
    unsigned int* led = (unsigned int*)0xc000fe00;
	mpz_t d;
	mpz_t p;
	mpz_t r;
	mpz_t res;
	mpz_t nm2;
	mpz_t tmp;
	mpz_init (d);
	mpz_init (p);
	mpz_init (r);
	mpz_init (res);
	mpz_init (nm2);
	mpz_init (tmp);

	if (mpz_cmp_ui(n, 1) == 0) { printf("a"); return 0; }
	if (mpz_cmp_ui(n, 2) == 0) { printf("b"); return 1; }
	if (mpz_scan1(n,0) != 0) { printf("c %d",mpz_scan1(n,0)); return 0; }

	*led = 0;
	mpz_sub_ui(d, n, 1);
	unsigned int s = mpz_scan0(d,0); 
	//unsigned long aa = mpz_get_ui(d);
	mpz_get_str(_pbuf, 16, n);
	printf("d %s\n",_pbuf);
	if (s > 0) {
		mpz_fdiv_q_2exp(tmp, d, s); // rsh
		mpz_init_set(d, tmp);
	}
	mpz_sub_ui(nm2, n, 1);
	//unsigned int zz = (unsigned int)mpz_get_ui(nm2);
	//printf("s %d %x\n",s,zz);
	*led = 1;

	for (int j = 0; j < k; j++) {
		*led = 7;
		//mpz_urandomm(r, state, nm2);
		generate_random_under_n(r, nm2);
		mpz_add_ui(r, r, 1);
		mod_exp_bnum(r, d, n, res);
		*led = 1;

		//unsigned int aa = (unsigned int)mpz_get_ui(r);
		//unsigned int zz = (unsigned int)mpz_get_ui(res);
		//printf("res %d %08x %08x\n",j,aa,zz);

		if (mpz_cmp_ui(res, 1) != 0) {
			int flg = 1;
			mpz_init_set(p, d);
			//unsigned int zz = (unsigned int)mpz_get_ui(p);
			//unsigned int aa = (unsigned int)mpz_get_ui(d);
			//printf("p 1st %08x %08x\n",aa,zz);
			for (int i = 0; i < s; i++) {
				//unsigned int zz = (unsigned int)mpz_get_ui(p);
				//printf("p %08x\n",zz);
				*led = i;
				mod_exp_bnum(r, p, n, res);
				if (mpz_cmp_ui(res, 1) == 0) {
					printf("flg<=0\n");
					flg = 0;
					break;
				}
				printf("flg<=1\n");
				mpz_mul_2exp(p, p, 1);
			}
			if (flg == 1) { return 0; }
		}
	}
	return 1;
}

// get prime
//int get_prime(int blen, gmp_randstate_t state, mpz_t n) {
int get_prime(int blen, mpz_t n) {
	int cntr = 0;
	while(1) {
		cntr++;
		get_random(blen, n);
		//gmp_printf("%Zx\n", n);
		mpz_get_str(_pbuf, 16, n);
		printf("candidate %s\n",_pbuf);
		//unsigned int zz = (unsigned int)mpz_get_ui(n);
		//printf("%08x\n",zz);

		if (mr_primary_test(100, n) == 1) {
		//if (mr_primary_test(4, state, n) == 1) {
			printf("%d It is prime!!!\n",cntr);
			break;
		} else {
			printf("%d It is not prime...\n",cntr);
		}	
	}	
	return 0;
}

//extern char _init_end;
//extern char _text_start;
//extern char _text_end;
//extern char _rodata_start;
//extern char _rodata_end;
//extern char _data_start;
//extern char _data_end;
//extern char _bss_start;
//extern char _bss_end;
//extern char _stack_start;
//extern char _stack_end;

int main () {
    unsigned int* led = (unsigned int*)0xc000fe00;

	//printf("_init_end %x\n",&_init_end); 
	//printf("_text_start %x\n",&_text_start); 
	//printf("_text_end %x\n",&_text_end); 
	//printf("_rodata_start %x\n",&_rodata_start); 
	//printf("_rodata_end %x\n",&_rodata_end); 
	//printf("_data_start %x\n",&_data_start); 
	//printf("_data_end %x\n",&_data_end); 
	//printf("_bss_start %x\n",&_bss_start); 
	//printf("_bss_end %x\n",&_bss_end); 
	//printf("_heap_start %x\n",&_heap_start); 
	//printf("_heap_end %x\n",&_heap_end); 
	//printf("_stack_start %x\n",&_stack_start); 
	//printf("_stack_end %x\n",&_stack_end); 

	//pass();
	*led = 0;
	srand(0);
	//gmp_randstate_t state;
	*led = 1;
	//gmp_randinit_default(state);
	*led = 2;
	//gmp_randseed_ui(state,0); wont work
	*led = 3;

	//mpz_t n;
	//mpz_init_set_str(n,"970eb807",16);
	//mpz_init_set_str(n,"970eb805",16);
	//mpz_init_set_str(n,"fefffffffefeffeffefefefefffeffff",16);
	//mpz_init_set_str(n,"f13dab851820cd98e20e9915790a10cdc0b11b41a24b992a3313003f423fa68fe340434fb088e9c4e90a7a84752f5faa83f58f374e2b06003cfc90004d431302b8e34a4d2846d3bb8fbb9dca7cebbb3d151a1c10a2b447903edbec11b30f758c8686a0ccf59b3e7fecab0ac32b339fe237a027b5da854c6dab8b5f0a37434b6ded1e5958a8ed55d67b02e0b6c7d3de869c6fcd6c4e77ad22f25790afdd45c4472edd470153e3a2ef34cee3bacbaef779d56a842ae61af7db37c3180b4bb5cd400559801a027dfa49ec8f39c74210b5b2cbc00e24b445673bcebeb9d06555706d7d832258fac400244f008d8c6adc10b8730a6631149a8afa239553c827146dc3",16);
	//gmp_printf("%Zx\n", n);
	//char* _pbuf =  mpz_get_str(NULL, 16, n);
	//printf("%s\n",_pbuf);
	//unsigned int zz = mr_primary_test(100, n);
	//printf("%d\n",zz);

	*led = 4;
	mpz_t n;
	mpz_init (n);
	*led = 5;
	get_prime(BLEN, n);
	*led = 6;
	printf("PRIME\n");
	//gmp_printf("%Zx\n", n);
	mpz_get_str(_pbuf, 16, n);
	printf("%s\n",_pbuf);
	*led = 7;

	pass();
	return 0;
}

