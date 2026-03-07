#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "/opt/riscv32i/gmp/include/gmp.h"
//#include "mini-gmp.h"
//#include "mini-gmp.c"

#define LP 10
//#define LP 1000
#define LP2 200

#define BLEN 512
#define ILEN (512/32)

#include "../add_for_cmpl_all.c"
#include "../add_for_cmpl2.c"


char _pbuf[4096];

// --- only for libgmp ---

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


// 簡易的なビット乱数生成（mini-gmp用）
void mpz_urandomb_custom(mpz_t rop, size_t bits) {
	unsigned int rnd = 0;
	mpz_t tmp;
	mpz_init (tmp);
    mpz_set_ui(rop, rand());
    for (size_t i = 0; i < bits; i += 30) { // rand()は通常15bit以上なので30bitずつ
        mpz_mul_2exp(tmp, rop, 30);
		mpz_init_set(rop, tmp);
		rnd = rand() & 0x3FFFFFFF;
		while ((rnd & 0xffffff) < 0xffff) {
			rnd = rand() & 0x3FFFFFFF;
		}
		//printf("rand %x\n",rnd);
        mpz_add_ui(tmp, rop, rnd);
		mpz_init_set(rop, tmp);
    }
    // 指定ビット数でマスク
    mpz_t mask;                
    mpz_init(mask);
    mpz_ui_pow_ui(mask, 2, bits);
    mpz_sub_ui(tmp, mask, 1);
	mpz_init_set(mask, tmp);
    mpz_and(tmp, rop, mask);
	mpz_init_set(rop, tmp);
    mpz_clear(mask);
    mpz_clear(tmp);
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

int get_random(unsigned int blen, mpz_t rnd) {

	//mpz_init (rnd);
	//mpz_urandomb(rnd, state, blen);
	mpz_urandomb_custom(rnd, blen);
	mpz_setbit(rnd,0);
	mpz_setbit(rnd,blen-1);
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
	mpz_t nm1;
	mpz_t tmp;
	mpz_init (d);
	mpz_init (p);
	mpz_init (r);
	mpz_init (res);
	mpz_init (nm1);
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
	mpz_sub_ui(nm1, n, 1);
	//unsigned int zz = (unsigned int)mpz_get_ui(nm1);
	//printf("s %d %x\n",s,zz);
	*led = 1;

	for (int j = 0; j < k; j++) {
		*led = 7;
		//mpz_urandomm(r, state, nm1);
		generate_random_under_n(r, nm1);
		mpz_get_str(_pbuf, 16, r);
		printf("r %s\n",_pbuf);
		mpz_add_ui(tmp, r, 1);
		mpz_init_set(r, tmp);
		mpz_powm(res, r, d, n);
		*led = j;

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
				mpz_powm(res, r, p, n);
				if (mpz_cmp_ui(res, 1) == 0) {
					printf("flg<=0\n");
					flg = 0;
					break;
				}
				printf("flg<=1\n");
				mpz_mul_2exp(tmp, p, 1);
				mpz_init_set(p, tmp);
			}
			if (flg == 1) {
    			mpz_clear(d);
    			mpz_clear(p);
    			mpz_clear(r);
    			mpz_clear(res);
    			mpz_clear(nm1);
    			mpz_clear(tmp);
				return 0;
			}
		}
	}
    mpz_clear(d);
    mpz_clear(p);
    mpz_clear(r);
    mpz_clear(res);
    mpz_clear(nm1);
    mpz_clear(tmp);
	return 1;
}

// get prime
//int get_prime(int blen, gmp_randstate_t state, mpz_t n) {
int get_prime(int blen, int k, mpz_t n) {
	int cntr = 0;
	while(1) {
		cntr++;
		get_random(blen, n);
		//gmp_printf("%Zx\n", n);
		mpz_get_str(_pbuf, 16, n);
		printf("candidate %s\n",_pbuf);
		//unsigned int zz = (unsigned int)mpz_get_ui(n);
		//printf("%08x\n",zz);

		if (mr_primary_test(k, n) == 1) {
		//if (mr_primary_test(4, state, n) == 1) {
			printf("%d It is prime!!!\n",cntr);
			break;
		} else {
			printf("%d It is not prime...\n",cntr);
		}	
	}	
	return 0;
}

extern char _init_end;
extern char _text_start;
extern char _text_end;
extern char _rodata_start;
extern char _rodata_end;
extern char _data_start;
extern char _data_end;
extern char _bss_start;
extern char _bss_end;
extern char _stack_start;
extern char _stack_end;

int main () {
    unsigned int* led = (unsigned int*)0xc000fe00;

	printf("_init_end %x\n",&_init_end); 
	printf("_text_start %x\n",&_text_start); 
	printf("_text_end %x\n",&_text_end); 
	printf("_rodata_start %x\n",&_rodata_start); 
	printf("_rodata_end %x\n",&_rodata_end); 
	printf("_data_start %x\n",&_data_start); 
	printf("_data_end %x\n",&_data_end); 
	printf("_bss_start %x\n",&_bss_start); 
	printf("_bss_end %x\n",&_bss_end); 
	printf("_heap_start %x\n",&_heap_start); 
	printf("_heap_end %x\n",&_heap_end); 
	printf("_stack_start %x\n",&_stack_start); 
	printf("_stack_end %x\n",&_stack_end); 

	//pass();
	*led = 0;
	srand(4);
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
	get_prime(BLEN, 20, n);
	*led = 6;
	printf("PRIME\n");
	//gmp_printf("%Zx\n", n);
	mpz_get_str(_pbuf, 16, n);
	printf("%s\n",_pbuf);
    mpz_clear(n);
	*led = 7;

	pass();
	return 0;
}

