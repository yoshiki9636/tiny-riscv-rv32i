#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLEN 2048
#define ILEN (2048/32)

int get_random(unsigned int blen, unsigned int ilen, unsigned int flg, unsigned int* rnd) {
	for (int i = 0; i < ilen; i++) {
		unsigned int tmp = (unsigned int)rand(); 
		rnd[i] = (tmp << 16) ^ tmp;
	}
	// for changed to odd numer
	if (flg == 1) { rnd[0] = rnd[0] | 0x1; }
	// for delete upper bits
	unsigned int imod = blen % 32;

	if (imod > 0) {
		unsigned int mask = 0xffffffff;
		for (int i = 31; i > 31 - imod; i--) {
			mask = mask ^ (0x1 << i);
		}
		rnd[ilen-1] &= mask;
		// for setting 1 to MSB
		if (flg == 1) { rnd[ilen-1] |= (0x1 << (31 - imod)); }
	}
	else {
		// for setting 1 to MSB
		if (flg == 1) { rnd[ilen-1] |= 0x80000000; }
	}
		
	return 0;
}

int print_bnum(unsigned int ilen, unsigned int* bnum) {
	for (int i = ilen - 1; i >= 0; i--) {
		printf("%08x",bnum[i]);
	}
	printf("\n");
	return 0;
}

unsigned int add_bnum(unsigned int ilen, unsigned int* a, unsigned int* b, unsigned int* c) {
	unsigned int carry = 0;
	for (int i = 0; i < ilen; i++) {
		unsigned int tmp = a[i] + b[i] + carry;
		carry = ((a[i] != 0)&&(b[i] != 0)&&(tmp < b[i])) ? 1 :
				((a[i] == 0)&&(b[i] == 0)) ? 0 :
		        (tmp == 0) ? 1 :
                ((a[i] != 0)&&(tmp == b[i])) ? 1 : 0;
		c[i] = tmp;
	}
	return carry;
}

unsigned int sub_bnum(unsigned int ilen, unsigned int* a, unsigned int* b, unsigned int* c) {
	unsigned int carry = 0;
	for (int i = 0; i < ilen; i++) {
		//c[i] = a[i] - b[i] - carry;
		//carry = (((b[i] !=0)||(carry ==1))&&(c[i] >= a[i])) ? 1 : 0;
		unsigned int tmp = a[i] - b[i] - carry;
		carry = (((b[i] !=0)||(carry ==1))&&(tmp >= a[i])) ? 1 : 0;
		c[i] = tmp;
	}
	return carry;
}

unsigned int inc_bnum(unsigned int ilen, unsigned int* a, unsigned int* c) {
	unsigned int carry = 0;
	for (int i = 0; i < ilen; i++) {
		unsigned int tmp = (i == 0) ? 1 : 0;
		unsigned int tmp2 = a[i] + tmp + carry;
		carry = ((a[i] != 0)&&(tmp != 0)&&(tmp2 < tmp)) ? 1 :
				((a[i] == 0)&&(tmp == 0)) ? 0 :
		        (tmp2 == 0) ? 1 :
                ((a[i] != 0)&&(tmp2 == tmp)) ? 1 : 0;
		c[i] = tmp2;
		//c[i] = a[i] + tmp + carry;
		//carry = ((a[i] != 0)||(tmp != 0)&&(c[i] <= a[i])) ? 1 : 0;
		//carry = ((a[i] != 0)&&(tmp != 0)&&(c[i] < tmp)) ? 1 :
				//((a[i] == 0)&&(tmp == 0)) ? 0 :
		        //(c[i] == 0) ? 1 :
                //((a[i] != 0)&&(c[i] == tmp)) ? 1 : 0;
	}
	return carry;
}

unsigned int dec_bnum(unsigned int ilen, unsigned int* a, unsigned int* c) {
	unsigned int carry = 0;
	for (int i = 0; i < ilen; i++) {
		unsigned int tmp = (i==0) ? 1 : 0;
		unsigned int tmp2 = a[i] - tmp - carry;
		carry = (((tmp !=0)||(carry ==1))&&(tmp2 >= a[i])) ? 1 : 0;
		c[i] = tmp2;
		//c[i] = a[i] - tmp - carry;
		//carry = (((tmp !=0)||(carry ==1))&&(c[i] >= a[i])) ? 1 : 0;
	}
	return carry;
}

// cは2倍の大きさ
unsigned int mul_bnum(unsigned int ilen, unsigned int* a, unsigned int* b, unsigned int* c) {
	for (int i = 0; i < ilen*2; i++) {
		c[i] = 0;
	}
	unsigned int overflow = 0;
	for (int j = 0; j < ilen; j++) {
		for (int i = 0; i < ilen; i++) {
			if (( a[j] == 0)||( b[i] == 0)) {
				// no addition for zero
				continue;
			}
			unsigned long long tmp = (unsigned long long)a[j] * (unsigned long long)b[i];
			unsigned int hi_tmp = (unsigned int)(tmp >> 32);
			unsigned int lo_tmp = (unsigned int)(tmp & 0x00000000ffffffffUL);
			if(((i+j+1 >= ilen * 2)&&(hi_tmp > 0))||((i+j >= ilen * 2)&&(lo_tmp > 0))) {
				overflow = 1;
				printf("ov chk1 %d %d\n",i,j);
				break;
			}
			else if(i+j < ilen * 2) {
				unsigned int ltmp = c[j+i] + lo_tmp;
				unsigned int carry = ((c[i+j] != 0)&&(lo_tmp != 0)&&(ltmp <= lo_tmp)) ? 1 : 0;
				c[j+i] = ltmp;
				if ((j+i+1 >= ilen * 2)&&((hi_tmp != 0)||(carry != 0)))  { 
					overflow = 1;
					printf("ov chk2 %d %d\n",i,j);
					break;
				}
				unsigned int htmp = c[j+i+1] + hi_tmp + carry;
				carry = ((c[i+j+1] != 0)&&(hi_tmp != 0)&&(htmp < hi_tmp)) ? 1 :
						((c[i+j+1] == 0)&&(hi_tmp == 0)) ? 0 :
				        (htmp == 0) ? 1 :
                        ((c[i+j+1] != 0)&&(htmp == hi_tmp)) ? 1 : 0;
				c[j+i+1] = htmp;
				for (int k = i + j + 2; k < ilen * 2; k++) {
					if (carry == 1) {
						c[k] += 1;
						carry = (c[k] == 0) ? 1 : 0;
					}		
				}		
				if (carry == 1) {
					overflow = 1;
					printf("ov chk3 %d %d\n",i,j);
					break;
				}
			}
		}
		if (overflow == 1) { break; }
	}
	return overflow;
}

unsigned int clear_bnum(unsigned int ilen, unsigned int* a) {
	for (int i = 0; i < ilen; i++) {
		a[i] = 0;
	}
    return 0;
}

unsigned int copy_bnum(unsigned int ilen, unsigned int* a, unsigned int* b) {
	memcpy(b, a, sizeof(unsigned int)*ilen);
    return 0;
}

unsigned int less_bnum(unsigned int ilen, unsigned int* a, unsigned int* b) {
	for (int i = ilen - 1; i >= 0; i--) {
		if (a[i] < b[i]) return 1;
		if (a[i] > b[i]) return 0;
	}
    return 0;
}

unsigned int is_zero_bnum(unsigned int ilen, unsigned int* a) {
	for (int i = 0; i < ilen; i++) {
		if (a[i] != 0) return 0;
	}
    return 1;
}

unsigned int is_snum_bnum(unsigned int ilen, unsigned int* a, unsigned int n) {
	for (int i = 1; i < ilen; i++) {
		if (a[i] != 0) return 0;
	}
	if (a[0] != n) return 0;

    return 1;
}

unsigned int lsh_bnum(unsigned int ilen, unsigned int* a, unsigned int* c) {
	unsigned int carry = 0;
	for (int i = 0; i < ilen; i++) {
		// tmp is for condition of c === a
		unsigned int tmp = a[i] >> 31;
		c[i] = (a[i] << 1) | carry;
		carry = tmp;
	}
	return carry;
}

unsigned int rsh_bnum(unsigned int ilen, unsigned int* a, unsigned int* c) {
	unsigned int carry = 0;
	for (int i = ilen - 1; i >= 0 ; i--) {
		// tmp is for condition of c === a
		unsigned int tmp = a[i] & 0x1;
		c[i] = (a[i] >> 1) | (carry << 31);
		carry = tmp;
	}
	return carry;
}

unsigned int div_bnum(unsigned int blen, unsigned int ilen, unsigned int* a, unsigned int* b, unsigned int* d, unsigned int* r) {
	unsigned int tmp2[ilen];

	clear_bnum(ilen, d);
	clear_bnum(ilen, r);
	copy_bnum(ilen, a, tmp2);

	int flg = 1;
	for (int i = 0; i < ilen; i++) {
		flg = ((flg == 1)&&(b[i] == 0)) ? 1 : 0;
	}
	// div by zero
	if (flg == 1) { return 0xffffffff; }

	for (int i = blen - 1; i >= 0; i--) {
		lsh_bnum(ilen, r, r);
		r[0] |= lsh_bnum(ilen, tmp2, tmp2);

		lsh_bnum(ilen, d, d);
		if (!less_bnum(ilen, r, b)) {
			sub_bnum(ilen, r, b, r);
			d[0] |= 1;
		}
	}
	return 0;
}

unsigned int mod_bnum(unsigned int blen, unsigned int ilen, unsigned int* a, unsigned int* b, unsigned int* r) {
	unsigned int d[ilen];
	unsigned int tmp2[ilen];

	for (int i = 0; i < ilen; i++) {
		d[i] = 0;
		r[i] = 0;
		tmp2[i] = a[i];
	}
	int flg = 1;
	for (int i = 0; i < ilen; i++) {
		flg = ((flg == 1)&&(b[i] == 0)) ? 1 : 0;
	}
	// div by zero
	if (flg == 1) { return 0xffffffff; }

	for (int i = blen - 1; i >= 0; i--) {
		lsh_bnum(ilen, r, r);
		r[0] |= lsh_bnum(ilen, tmp2, tmp2);

		lsh_bnum(ilen, d, d);
		if (!less_bnum(ilen, r, b)) {
			sub_bnum(ilen, r, b, r);
			d[0] |= 1;
		}
	}
	return 0;
}

// r = (a * b) % m
// tmpm, tmpr, tmp1, tmp2, dummy: [ilen*2]
unsigned int mul_mod_bnum(unsigned int blen, unsigned int ilen, unsigned int* a, unsigned int* b, unsigned int* m, unsigned int* r ) {
	unsigned int tmp1[ilen*2];
	unsigned int tmpm[ilen*2];
	unsigned int tmpr[ilen*2];
	clear_bnum(ilen*2, tmpm);
	copy_bnum(ilen, m, tmpm);

	mul_bnum(ilen, a, b, tmp1);
	mod_bnum(blen*2, ilen*2, tmp1, tmpm, tmpr);
	copy_bnum(ilen, tmpr, r);

	return 0;
}


// Modular Exponentiation: res = base^exp % mod
// tmp1, tmp2, tmp3, tmp4, dummy: [ilen*2]
unsigned int mod_exp_bnum(unsigned int blen, unsigned int ilen, unsigned int* base, unsigned int* exp, unsigned int* mod, unsigned int* res) {

	clear_bnum(ilen, res);
	res[0] = 1;

	unsigned int b[ilen];
	unsigned int e[ilen];
	copy_bnum(ilen, base, b);
	copy_bnum(ilen, exp, e);
	
	while(!is_zero_bnum(ilen, e)) {
		if (e[0] & 1) {
			mul_mod_bnum(blen, ilen, res, b, mod, res );
		}
		mul_mod_bnum(blen, ilen, b, b, mod, b );

		rsh_bnum(ilen, e, e);
	}
	return 0;
}


unsigned int mod_exp_r_bnum(unsigned int blen, unsigned int ilen, unsigned int* base, unsigned int* exp, unsigned int* mod, unsigned int* res ) {

	unsigned int ba[ilen];
	unsigned int ex[ilen];
	unsigned int mo[ilen];
	unsigned int re[ilen];
	copy_bnum(ilen, exp, ex);
	copy_bnum(ilen, mod, mo);
	mod_bnum(blen, ilen, base, mo, ba);
	//print_bnum(BLEN, ex);

	if (is_snum_bnum(ilen, ex, 0)) {
		//printf("EXP=0\n");
		clear_bnum(ilen, res);
		res[0] = 1;
		return 0;
	}
	else if (is_snum_bnum(ilen, ex, 1)) {
		//printf("EXP=1\n");
		copy_bnum(ilen, ba, res);
		return 0;
	}
	else if ((ex[0] & 1) == 0) {
		//printf("EXP\%2=0\n");
		rsh_bnum(ilen, ex, ex);
		mul_mod_bnum(blen, ilen, ba, ba, mo, re);
		mod_exp_r_bnum(blen, ilen, re, ex, mo, res);
		return 0;
	}
	else if ((ex[0] & 1) == 1) {
		//printf("EXP\%2=1\n");
		dec_bnum(ilen, ex, ex);
		mod_exp_r_bnum(blen, ilen, ba, ex, mo, re);
		mul_mod_bnum(blen, ilen, ba, re, mo, res);
		return 0;
	}
	else {
		printf("error!!!\n");
		return 1;
	}
}


// millar rabin primary test
unsigned int mr_primary_test(unsigned int blen, unsigned int ilen, unsigned int k, unsigned int* n, unsigned int* d, unsigned int* r, unsigned int* p, unsigned int* res) {

	if (is_snum_bnum(ilen, n, 1)) { return 0; }
	if (is_snum_bnum(ilen, n, 2)) { return 1; }
	if (( n[0] & 0x1) == 0) { return 0; }

	//unsigned int res2[ilen];

	dec_bnum(ilen, n, d);
	unsigned int s = 0;
	while (( d[0] & 0x1) == 1) {
	    rsh_bnum(ilen, d, d);
		s++;
	}

	for (int j = 0; j < k; j++) {
		get_random(blen, ilen, 0, r);
		if (less_bnum(ilen, n, r)) {
			sub_bnum(ilen, r, n, r);
		}

		mod_exp_bnum(blen, ilen, r, d, n, res);
		//mod_exp_r_bnum(blen, ilen, r, d, n, res);
		if (!is_snum_bnum(ilen, res, 1)) {
			int flg = 1;
			copy_bnum(ilen, d, p);
			for (int i = 0; i < s; i++) {
				lsh_bnum(ilen, p, p);
				mod_exp_bnum(blen, ilen, r, p, n, res);
				//mod_exp_r_bnum(blen, ilen, r, p, n, res);
				if (!is_snum_bnum(ilen, res, 1)) {
					flg = 0;
					break;
				}
			}
			if (flg == 1) { return 0; }
		}
	}
	return 1;
}

// get prime
unsigned int get_prime(unsigned int blen, unsigned int ilen, unsigned int* n, unsigned int* d, unsigned int* r, unsigned int* p, unsigned int* res ) {
	int cntr = 0;
	while(1) {
		cntr++;
		get_random(blen, ilen, 1, n);
		print_bnum(ilen, n);
		if (mr_primary_test(blen, ilen, 100, n, d, r, p, res) == 1) {
			printf("%d It is prime!!!\n",cntr);
			break;
		} else {
			printf("%d It is not prime...\n",cntr);
		}	
	}	
	return 0;
}



int main () {
	unsigned int ilen = BLEN / 32;
	unsigned int imod = BLEN % 32;
	if (imod > 0) { ilen++; }

	/*
	unsigned int rand1[ILEN];
	unsigned int rand2[ILEN];
	unsigned int rand3[ILEN*2];
	unsigned int rand4[ILEN];
	unsigned int rand5[ILEN];
	unsigned int ans3[ILEN];
	unsigned int rem4[ILEN];
	unsigned int tmp5[ILEN];
	unsigned int tmp6[ILEN];

	//unsigned int c = add_bnum(BLEN, rand1, rand2, rand3);
	//unsigned int c = sub_bnum(BLEN, rand2, rand1, rand3);
	for (int i = ILEN / 2; i < ILEN; i++) {
		rand1[i] = 0;
		rand2[i] = 0;
	}
	for (int i = 0; i < ILEN; i++) {
		rand1[i] = 0xffffffff;
		rand2[i] = 0xffffffff;
	}
	//rand1[ILEN/2-1] = 0xf0000000;
	//rand2[ILEN/2-1] = 0xf0000000;
	//rand1[0] = 0xffff;
	//rand2[0] = 0xffff;
	//inc_bnum(BLEN, rand1, rand4);
	lsh_bnum(BLEN, rand1, rand4);
	unsigned int x = less_bnum(BLEN, rand1, rand4);
	unsigned int y = mul_bnum(BLEN, rand4, rand1, rand3);
	print_bnum(BLEN, rand1);
	print_bnum(BLEN, rand4);
	print_bnum(BLEN*2, rand3);
	printf("%d\n",x);
	
	for (int i = 0; i < ILEN; i++) {
		rand1[i] = 0;
		rand2[i] = 0;
	}

	//rand1[1] = 0xffff;
	//rand2[0] = 7;
	get_random(BLEN, rand2);
	get_random(BLEN, rand1);
	for (int i = ILEN / 2; i < ILEN; i++) {
		rand2[i] = 0;
	}
    x = div_bnum(BLEN, rand1, rand2, ans3, rem4, tmp5, tmp6 );
	print_bnum(BLEN, rand1);
	print_bnum(BLEN, rand2);
	print_bnum(BLEN, ans3);
	print_bnum(BLEN, rem4);
	print_bnum(BLEN, tmp5);
	printf("%d\n",x);
	*/

	// moduler exp test
	unsigned int n[ILEN];
	unsigned int d[ILEN];
	unsigned int r[ILEN];
	unsigned int p[ILEN];
	unsigned int res[ILEN];
	//unsigned int b[ILEN];
	//unsigned int e[ILEN];
	//unsigned int tmp11[ILEN*2];
	//unsigned int tmp12[ILEN*2];
	//unsigned int tmp13[ILEN*2];
	//unsigned int tmp14[ILEN*2];
	//unsigned int dummy[ILEN*2];
	unsigned int rand[ILEN];

/*
rand[63]=0xf077481b;
rand[62]=0xa9745c6f;
rand[61]=0x3e25d238;
rand[60]=0x390bf034;
rand[59]=0x02b332ad;
rand[58]=0xfb9df0b4;
rand[57]=0x6214f87b;
rand[56]=0x287e3f97;
rand[55]=0x8a080ca9;
rand[54]=0x33864224;
rand[53]=0xa1d564b6;
rand[52]=0xb6601965;
rand[51]=0xe96c3baf;
rand[50]=0xfad5c072;
rand[49]=0xa897f763;
rand[48]=0xe759fd8c;
rand[47]=0xd80a03c4;
rand[46]=0x4204b608;
rand[45]=0xb6c15e34;
rand[44]=0xa4a42051;
rand[43]=0xc671b174;
rand[42]=0x215c7e8a;
rand[41]=0x73b87993;
rand[40]=0x70d29171;
rand[39]=0xf7749a7e;
rand[38]=0x8c6b69e8;
rand[37]=0xd272e5e3;
rand[36]=0x368e2f22;
rand[35]=0x330416b7;
rand[34]=0xc89ad767;
rand[33]=0x3e1f06c0;
rand[32]=0x1f51ba92;
rand[31]=0xeb3cde00;
rand[30]=0xbe3052bb;
rand[29]=0xda5dcafa;
rand[28]=0x08641a51;
rand[27]=0x03fd3c67;
rand[26]=0x609daba9;
rand[25]=0x47479ec7;
rand[24]=0xbcd589b4;
rand[23]=0x2ad70931;
rand[22]=0xc3d4cd6d;
rand[21]=0xef535c42;
rand[20]=0x6cf6592a;
rand[19]=0xa9d6591b;
rand[18]=0x64199e70;
rand[17]=0x644f8ac7;
rand[16]=0x64b5cb76;
rand[15]=0x094d7343;
rand[14]=0x2f66a721;
rand[13]=0x84ddbe45;
rand[12]=0x9c846aa8;
rand[11]=0x39520f74;
rand[10]=0x391483df;
rand[9]=0xb7d49b59;
rand[8]=0xf3f9ea82;
rand[7]=0x578392e2;
rand[6]=0x5598af12;
rand[5]=0x52fc8181;
rand[4]=0x7acfc22c;
rand[3]=0xd72092a8;
rand[2]=0x2bc73303;
rand[1]=0x99e3d3ad;
rand[0]=0x95ddd83b;
*/
	//unsigned int zz = mr_primary_test(BLEN, ilen, 100, rand, d, r, p, res);
	//printf("%d\n",zz);

	get_prime(BLEN, ilen, n, d, r, p, res);
	printf("PRIME\n");
	print_bnum(ilen, n);

	return 0;
}

