#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <gmp.h>
//#include "mini-gmp.h" // mini-gmp.h と mini-gmp.c を同じフォルダに配置してください

#define BLEN 2048
#define ILEN (2048/32)

// モンゴメリー乗算に必要なパラメータ
typedef struct {
    mpz_t n;    // 法 (n)
    mpz_t r;    // R = 2^k > n
    mpz_t r2;   // R^2 mod n
    mpz_t n_inv;// -n^-1 mod R
    int k;      // Rのビット数
} montgomery_context;

// モンゴメリー構造体の初期化
void montgomery_init(montgomery_context *ctx, const mpz_t n) {
    mpz_init_set(ctx->n, n);
    // n は奇数である必要がある
    ctx->k = mpz_sizeinbase(n, 2);
    mpz_init_set_ui(ctx->r, 1);
    mpz_mul_2exp(ctx->r, ctx->r, ctx->k); // R = 2^k
    
    // r2 = R^2 mod n
    mpz_init(ctx->r2);
    mpz_mul(ctx->r2, ctx->r, ctx->r);
    mpz_mod(ctx->r2, ctx->r2, ctx->n);
    
    // n_inv = -n^-1 mod R (ctx->rを使用)
    // 実際には 1/n mod R を計算し、Rから引く
    mpz_init(ctx->n_inv);
    mpz_invert(ctx->n_inv, ctx->n, ctx->r);
    mpz_sub(ctx->n_inv, ctx->r, ctx->n_inv);
    mpz_mod(ctx->n_inv, ctx->n_inv, ctx->r);
}

// モンゴメリーリダクション (T * R^-1 mod n)
void montgomery_reduce(mpz_t res, const mpz_t t, montgomery_context *ctx) {
    mpz_t m, tmp;
    mpz_init(m);
    mpz_init(tmp);
    
    // m = (T mod R) * n_inv mod R
    mpz_mod(m, t, ctx->r);
    mpz_mul(m, m, ctx->n_inv);
    mpz_mod(m, m, ctx->r);
    
    // tmp = (t + m * n) / R
    mpz_mul(tmp, m, ctx->n);
    mpz_add(tmp, tmp, t);
    mpz_tdiv_q_2exp(tmp, tmp, ctx->k);
    
    if (mpz_cmp(tmp, ctx->n) >= 0) {
        mpz_sub(res, tmp, ctx->n);
    } else {
        mpz_set(res, tmp);
    }
    
    mpz_clear(m);
    mpz_clear(tmp);
}

// モンゴメリー乗算 (a * b * R^-1 mod n)
void montgomery_mul(mpz_t res, const mpz_t a, const mpz_t b, montgomery_context *ctx) {
    mpz_t t;
    mpz_init(t);
    mpz_mul(t, a, b);
    montgomery_reduce(res, t, ctx);
    mpz_clear(t);
}

// モンゴメリー形式への変換 (a * R mod n)
void to_montgomery(mpz_t res, const mpz_t a, montgomery_context *ctx) {
    montgomery_mul(res, a, ctx->r2, ctx);
}

// モンゴメリー形式からの変換 (res = a_mont * R^-1 mod n)
void from_montgomery(mpz_t res, const mpz_t a_mont, montgomery_context *ctx) {
    //mpz_t one;
    //mpz_init_set_ui(one, 1);
    montgomery_reduce(res, a_mont, ctx);
    //mpz_clear(one);
}

// モンゴメリー乗算を用いたモジュロ指数演算
void montgomery_powm_inner(mpz_t res, const mpz_t base, const mpz_t exp, const mpz_t r_mont_1, montgomery_context *ctx) {
    mpz_t a, e, r_mont;
    mpz_init_set(a, base);
    mpz_init_set(e, exp);
    //mpz_init_set_ui(r_mont, 1);
    mpz_init_set(r_mont, r_mont_1);
    
    // 1をモンゴメリー空間に変換 (R mod n)
    //to_montgomery(r_mont, r_mont, ctx);
    
    // baseをモンゴメリー空間に変換
    //to_montgomery(a, a, ctx);
    
    while (mpz_sgn(e) > 0) {
        if (mpz_odd_p(e)) {
            montgomery_mul(r_mont, r_mont, a, ctx);
        }
        montgomery_mul(a, a, a, ctx);
        mpz_tdiv_q_2exp(e, e, 1);
    }
    
    // 結果を通常空間に戻す (res = r_mont * R^-1 mod n)
    //from_montgomery(res, r_mont, ctx);
    mpz_set(res, r_mont);
    
    mpz_clear(a);
    mpz_clear(e);
    mpz_clear(r_mont);
}
/*
// モンゴメリー乗算を用いたモジュロ指数演算
void montgomery_powm(mpz_t res, const mpz_t base, const mpz_t exp, montgomery_context *ctx) {
    mpz_t a, e, r_mont;
    mpz_init_set(a, base);
    mpz_init_set(e, exp);
    mpz_init_set_ui(r_mont, 1);
    
    // 1をモンゴメリー空間に変換 (R mod n)
    mpz_init_set_ui(r_mont, 1);
    to_montgomery(r_mont, r_mont, ctx);
    
    // baseをモンゴメリー空間に変換
    to_montgomery(a, a, ctx);
    
    while (mpz_sgn(e) > 0) {
        if (mpz_odd_p(e)) {
            montgomery_mul(r_mont, r_mont, a, ctx);
        }
        montgomery_mul(a, a, a, ctx);
        mpz_tdiv_q_2exp(e, e, 1);
    }
    
    // 結果を通常空間に戻す (res = r_mont * R^-1 mod n)
    from_montgomery(res, r_mont, ctx);
    
    mpz_clear(a);
    mpz_clear(e);
    mpz_clear(r_mont);
}
*/

// ミラーラビン素数判定
int is_prime_miller_rabin(const mpz_t n, gmp_randstate_t state, int k_iterations) {
    // 2以下の素数と偶数の処理
    if (mpz_cmp_ui(n, 2) < 0) return 0;
    if (mpz_cmp_ui(n, 2) == 0) return 1;
    if (mpz_even_p(n)) return 0;
    
    mpz_t r_mont_1;
    mpz_init_set_ui(r_mont_1, 1);

	montgomery_context ctx;
   	montgomery_init(&ctx, n);

   	to_montgomery(r_mont_1, r_mont_1, &ctx);

    mpz_t d, n_minus_1, a, x, s_mpz;
    mpz_init_set(d, n);
    mpz_sub_ui(d, d, 1);
    mpz_init_set(n_minus_1, d);
   	to_montgomery(n_minus_1, n_minus_1, &ctx);
    int s = mpz_scan0(d,0);

    if (s > 0) {
        mpz_fdiv_q_2exp(d, d, s); // rsh
    }
    
    mpz_inits(a, x, s_mpz, NULL);
    
    for (int i = 0; i < k_iterations; i++) {
        // a を [2, n-2] からランダムに選択
        mpz_sub_ui(x, n, 4);
        mpz_urandomm(a, state, x);
        mpz_add_ui(a, a, 2);
    	to_montgomery(a, a, &ctx);
        
        // x = a^d mod n (モンゴメリー乗算を使用)
        montgomery_powm_inner(x, a, d, r_mont_1, &ctx);
        
        if (mpz_cmp(x, r_mont_1) == 0 || mpz_cmp(x, n_minus_1) == 0)
        //if (mpz_cmp_ui(x, 1) == 0 || mpz_cmp(x, n_minus_1) == 0)
            continue;
        
        int composite = 1;
        for (int r = 1; r < s; r++) {
            // x = x^2 mod n
            //montgomery_mul(x, x, x, &ctx); // モンゴメリー空間での自乗
            // 実際はここもmontgomery_reduceを呼ぶ必要があるが、mul(x,x)で代用
            // 正確にはto_montgomery/from_montgomeryの管理が必要
            // 簡易的にmontgomery_mulで実装
            montgomery_reduce(x, x, &ctx); // 修正: 正しいモンゴメリー自乗

            if (mpz_cmp(x, n_minus_1) == 0) {
                composite = 0;
                break;
            }
        }
        
        if (composite) {
            mpz_clears(d, n_minus_1, a, x, s_mpz, NULL);
    		mpz_clear(r_mont_1);
            // 構造体のクリア処理が必要
            return 0; // 合成数
        }
    }
    
    mpz_clears(d, n_minus_1, a, x, s_mpz, NULL);
    mpz_clear(r_mont_1);
    return 1; // 確率的素数
}


int get_random(int blen, gmp_randstate_t state, mpz_t rnd) {

	mpz_init (rnd);
	mpz_urandomb(rnd, state, blen);
	mpz_setbit(rnd,0);
	mpz_setbit(rnd,blen-1);
	return 0;
}

// get prime
int get_prime(int blen, gmp_randstate_t state, mpz_t n) {

	int cntr = 0;
	while(1) {
		cntr++;
		get_random(blen, state, n);
		gmp_printf("%Zx\n", n);


		//if (mr_primary_test(100, state, n) == 1) {
    	if (is_prime_miller_rabin(n, state, 100)) {
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

