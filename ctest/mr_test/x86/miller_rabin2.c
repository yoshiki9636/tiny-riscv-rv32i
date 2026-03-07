#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <gmp.h>

#define BLEN 2048
#define ILEN (2048/32)

int get_random(int blen, gmp_randstate_t state, mpz_t rnd) {

	mpz_init (rnd);
	mpz_urandomb(rnd, state, blen);
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
	return 0;
}

// Modular Exponentiation: res = base^exp % mod
int mod_exp_bnum(mpz_t base, mpz_t exp, mpz_t mod, mpz_t res) {
	mpz_t b;
	mpz_t e;
	mpz_set_ui(res, 1);
	mpz_init_set(b, base);
	mpz_init_set(e, exp);

	while(mpz_sgn(e) != 0) {
		if (mpz_scan1(e,0) == 0) {
			mul_mod_bnum(res, b, mod, res);
		}
		mul_mod_bnum(b, b, mod, b);
		mpz_fdiv_q_2exp (e, e, 1); // rsh
	}
    unsigned int zz = (unsigned int)mpz_get_ui(res);
    unsigned int aa = (unsigned int)mpz_get_ui(base);
    unsigned int bb = (unsigned int)mpz_get_ui(exp);
    unsigned int cc = (unsigned int)mpz_get_ui(mod);
    printf("me %08x %08x %08x %08x\n",aa,bb,cc,zz);

	return 0;
}


// millar rabin primary test
int mr_primary_test(int k, gmp_randstate_t state, mpz_t n) {
//int mr_primary_test(int k, gmp_randstate_t state, unsigned int nn) {
	//mpz_t n;
	//mpz_init (n);
	//mpz_set_ui(n, nn);
	mpz_t d;
	mpz_t p;
	mpz_t r;
	mpz_t res;
	mpz_t nm2;
	mpz_init (d);
	mpz_init (p);
	mpz_init (r);
	mpz_init (res);
	mpz_init (nm2);

	if (mpz_cmp_ui(n, 1) == 0) { printf("a"); return 0; }
	if (mpz_cmp_ui(n, 2) == 0) { printf("b"); return 1; }
	if (mpz_scan1(n,0) != 0) { printf("c"); return 0; }

	mpz_sub_ui(d, n, 1);
	unsigned int s = mpz_scan0(d,0);
	//unsigned int s = mpz_scan1(d,0);
	mpz_fdiv_q_2exp(d, d, s); // rsh
	mpz_sub_ui(nm2, n, 2);

	for (int j = 0; j < k; j++) {
		mpz_urandomm(r, state, nm2);
		mpz_add_ui(r, r, 1);
		//mod_exp_bnum(r, d, n, res);
		mpz_powm(res, r, d, n);

		if (mpz_cmp_ui(res, 1) != 0) {
			int flg = 1;
			mpz_init_set(p, d);
			for (int i = 0; i < s; i++) {
				//mod_exp_bnum(r, p, n, res);
				mpz_powm(res, r, p, n);
				if (mpz_cmp_ui(res, 1) == 0) {
					flg = 0;
					break;
				}
				mpz_mul_2exp(p, p, 1);
			}
			if (flg == 1) { return 0; }
		}
	}
	return 1;
}


// millar rabin primary test
int mr_primary_test2(int k, gmp_randstate_t state, unsigned int nn) {
	mpz_t n;
	mpz_init (n);
	mpz_set_ui(n, nn);
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
	if (mpz_scan1(n,0) != 0) { printf("c"); return 0; }

	mpz_sub_ui(d, n, 1);
	unsigned int s = mpz_scan0(d,0); 
	unsigned long aa = mpz_get_ui(d);
	printf("d %lx\n",aa);
	if (s > 0) {
		mpz_fdiv_q_2exp(tmp, d, s); // rsh
		mpz_init_set(d, tmp);
	}
	mpz_sub_ui(nm2, n, 1);
	unsigned int zz = (unsigned int)mpz_get_ui(nm2);
	printf("s %d %x\n",s,zz);

	for (int j = 0; j < k; j++) {
		mpz_urandomm(r, state, nm2);
		mpz_add_ui(r, r, 1);
		mod_exp_bnum(r, d, n, res);

		unsigned int zz = (unsigned int)mpz_get_ui(res);
		printf("rand %08x\n",zz);

		if (mpz_cmp_ui(res, 1) != 0) {
			int flg = 1;
			mpz_init_set(p, d);
			unsigned int zz = (unsigned int)mpz_get_ui(p);
			unsigned int aa = (unsigned int)mpz_get_ui(d);
			printf("p 1st %08x %08x\n",aa,zz);
			for (int i = 0; i < s; i++) {
				unsigned int zz = (unsigned int)mpz_get_ui(p);
				printf("p %08x\n",zz);
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
int get_prime(int blen, gmp_randstate_t state, mpz_t n) {
	int cntr = 0;
	while(1) {
		cntr++;
		get_random(blen, state, n);
		gmp_printf("%Zx\n", n);

		if (mr_primary_test(20, state, n) == 1) {
			printf("%d It is prime!!!\n",cntr);
			break;
		} else {
			printf("%d It is not prime...\n",cntr);
		}	
	}	
	return 0;
}

int main () {

	gmp_randstate_t state;
	gmp_randinit_default(state);
	gmp_randseed_ui(state,5);

	//mpz_t n;
	//mpz_init_set_str(n,"f13dab851820cd98e20e9915790a10cdc0b11b41a24b992a3313003f423fa68fe340434fb088e9c4e90a7a84752f5faa83f58f374e2b06003cfc90004d431302b8e34a4d2846d3bb8fbb9dca7cebbb3d151a1c10a2b447903edbec11b30f758c8686a0ccf59b3e7fecab0ac32b339fe237a027b5da854c6dab8b5f0a37434b6ded1e5958a8ed55d67b02e0b6c7d3de869c6fcd6c4e77ad22f25790afdd45c4472edd470153e3a2ef34cee3bacbaef779d56a842ae61af7db37c3180b4bb5cd400559801a027dfa49ec8f39c74210b5b2cbc00e24b445673bcebeb9d06555706d7d832258fac400244f008d8c6adc10b8730a6631149a8afa239553c827146dc3",16);
	//gmp_printf("%Zx\n", n);
	//unsigned int zz = mr_primary_test(100, state, n);
	//printf("%d\n",zz);
	//printf("%d\n", mr_primary_test(100, state, 0xb9bca875));
	//printf("%d\n", mr_primary_test(100, state, 0x82902d2f));
	//printf("%d\n", mr_primary_test(100, state, 0xe80bbdc9));
	//printf("%d\n", mr_primary_test(100, state, 0xd84a873b));
	//printf("%d\n", mr_primary_test(100, state, 0xbf767863));
	//printf("%d\n", mr_primary_test(100, state, 0xaa3dc18b));
	//printf("%d\n", mr_primary_test(100, state, 0xfa337237));
	//printf("%d\n", mr_primary_test(100, state, 0xe846aeb7));
	//printf("%d\n", mr_primary_test(100, state, 0xe694ce75));
	//printf("%d\n", mr_primary_test(100, state, 0xa88b3e55));
	//printf("%d\n", mr_primary_test(100, state, 0xcc904dcf));
	//printf("%d\n", mr_primary_test(100, state, 0xb79bece7));
	//printf("%d\n", mr_primary_test(100, state, 0xfd530069));

	//printf("%d\n", mr_primary_test(100, state, 0xb59a46e7));
	//printf("%d\n", mr_primary_test(100, state, 0xde0bbadf));
	//printf("%d\n", mr_primary_test(100, state, 0xdc732865));
	//printf("%d\n", mr_primary_test(100, state, 0xeeba0917));
	//printf("%d\n", mr_primary_test(100, state, 0xb14b747f));
	//printf("%d\n", mr_primary_test(100, state, 0x83e49377));
	//printf("%d\n", mr_primary_test(100, state, 0xd5c3f27b));
	//printf("%d\n", mr_primary_test2(100, state, 0xfd9d0d33));
	//printf("%d\n", mr_primary_test2(100, state, 0xe4150a6d));
	//printf("%d\n", mr_primary_test2(100, state, 0x970eb807)); // prime!!
	//printf("%d\n", mr_primary_test2(100, state, 0x859e499b));
	//printf("%d\n", mr_primary_test(100, state, 0x970eb807)); // prime!!

	//mpz_t base;
	//mpz_t exp;
	//mpz_t mod;
	//mpz_t res;
	//mpz_init (base);
	//mpz_init (exp);
	//mpz_init (mod);
	//mpz_init (res);
	//mpz_set_ui(base, 0x0e9c4dff);
	//mpz_set_ui(exp, 0x970eb806);
	//mpz_set_ui(mod, 0x970eb807);

	//mod_exp_bnum( base, exp, mod, res);
	//gmp_printf("%Zx\n", res);

	mpz_t n;
	mpz_init (n);
	get_prime(BLEN, state, n);
	printf("PRIME\n");
	gmp_printf("%Zx\n", n);

	return 0;
}

