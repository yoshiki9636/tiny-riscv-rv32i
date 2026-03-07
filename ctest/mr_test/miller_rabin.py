import random

def modular_exp(a, b, n):
    res = 1
    while b != 0:
        if b & 1 != 0:
            res = (res * a) % n
        a = (a * a) % n
        b = b >> 1
    return res

def gen_rand(bit_length):
    bits = [random.randint(0,1) for _ in range(bit_length - 2)]
    ret = 1
    for b in bits:
        ret = ret * 2 + int(b)
    return ret * 2 + 1

def mr_primary_test(n, k=100):
    if n == 1:
        return False
    if n == 2:
        return True
    if n % 2 == 0:
        return False
    d = n - 1
    s = 0
    while d % 2 != 0:
        d /= 2
        s += 1

    r = [random.randint(1, n - 1) for _ in range(k)]
    for a in r:
        if modular_exp(a, d, n) != 1:
            pl = [(2 ** rr) * d for rr in range(s)]
            flg = True
            for p in pl:
                if modular_exp(a, p, n) == 1:
                    flg = False
                    break
            if flg:
                return False
    return True

def gen_prime(bit):
    cntr = 0
    while True:
        cntr += 1
        ret = gen_rand(bit)
        #print(ret)
        print(hex(ret))
        if mr_primary_test(ret):
            print(cntr," It is prime!!!!")
            break
        else:
            print(cntr," It is not prime...")
    return ret

#print(mr_primary_test(0x82f5aafa1f3f9120a8435a1f509028a8941ae76c643146c8894330836e3e410c752027afe6c7ffaf9a4ad676bd2657e7c2f3a53e43bbb2ebd4a2eb75017880a78a3a186ad24276aabf60cea832607ea1c477e1aa24c694396f7f3e748a189eada2865e07c215634f2db22620415ab17d4b55086f2f451e299cfbdbb6d663265d17aedf2f3aad2a95ce2235bbfe6c342ec36c1d899fd327d08073ce41abc0b98154911f5b46a6aebadcf2d52eab76fa35be9f56eb61b32dd89288c83d6014a2239d9e1a5aada33ed3375bbd4efe577cfb896745d088eaa1ba52bbd85ee0bd3d65f734041e07b616ad02146eb8c6684e32b6dea62c42e96fb41c731b72f39d0821))
#print(mr_primary_test(0xa1cd39fd23e3e7c9e03bc705928365c9606ad6b1253006f675923aad0dbeee604cd29a567060b5a1f834b7539a84c34e15cd02324b3acb7f1698271d8cea5dc4ae013eff917ee2536c06295d73887ae5414b982a4656f5ab3f331452c8f6f563a40d2af636263b011b307a412dd2c87697c182eddae2f97ec9a23cbb28da8e0bd0a5cb7ef0416e5d212ea6b6a334de68d0419749b3d51bc32c0dc2db5e5f96aa41e65e8ece9dcd7fdffba27d02ba7421cb030c2e2d01c24e1d74acdae59960bbee5534304bc80ed839fe4898dff404703ebf0af14e4e7e93a0b28dbb6abee757db2c2d448f8349fafb7827db477e288f01c155c316eafc09b2c8a5700711af19))
#print(mr_primary_test(0xe1c24f0f7311cc4396e8d4913c3c3a9b3132a00f837a48e46c70c404392041b3d8a49083fa689d148152b2fa817191854218cbc694fc2b97c2e7d294ef0cea933753b5fa3be5d0cac2f6c8df64caebc9956d48c7527eeb9076c8d559bba64f3c861997ec3b522947408f97950340498929e78bba94a51e4828c20e4893cc2c4d1610ade7f20583eaf42f60aefcd3962dd410e47f2802ee2bc5370ec8b963b72c1874666fd8555f6e022d6754ed43b132e8616f77cd477e4cb312812a58d526dcd2f0bb79aee5c837d6528e177f1d857a5a772837460174b1d8a322fc13e862faed4351b73f9476996ca31543419f37bab47925d23d6a41a0c42f44843fcd5109))
#print(mr_primary_test(0xf13dab851820cd98e20e9915790a10cdc0b11b41a24b992a3313003f423fa68fe340434fb088e9c4e90a7a84752f5faa83f58f374e2b06003cfc90004d431302b8e34a4d2846d3bb8fbb9dca7cebbb3d151a1c10a2b447903edbec11b30f758c8686a0ccf59b3e7fecab0ac32b339fe237a027b5da854c6dab8b5f0a37434b6ded1e5958a8ed55d67b02e0b6c7d3de869c6fcd6c4e77ad22f25790afdd45c4472edd470153e3a2ef34cee3bacbaef779d56a842ae61af7db37c3180b4bb5cd400559801a027dfa49ec8f39c74210b5b2cbc00e24b445673bcebeb9d06555706d7d832258fac400244f008d8c6adc10b8730a6631149a8afa239553c827146dc3))
#print(mr_primary_test(0xb9bca875))
print(mr_primary_test(0xa008a02b))
print(mr_primary_test(0x92176231))
print(mr_primary_test(0xaf3c7831))
print(mr_primary_test(0xd8da31cd))
print(mr_primary_test(0xbd67cce6b1c9a05bdde1d32fb32282f9))
print(mr_primary_test(0xcd89715e79af804c6fcd1cbfc5437c87))
print(mr_primary_test(0xfffffffc0ef8ae79))
print(mr_primary_test(0x9000931a548b026f))
print(mr_primary_test(0xf22419e9c89067a722419e9c89067a73))
print(mr_primary_test(0xaf3db48abcf6d22af3db48abcf6d22af3db48abcf6d22af3db48abcf6d22af3d))
print(mr_primary_test(0xec52f559b14bd566c52f559b14bd566c52f559b14bd566c52f559b14bd566c52f559b14bd566c52f559b14bd566c52f559b14bd566c52f559b14bd566c52f559))
print(mr_primary_test(0xe791584b67f7c6954362ad871f49f09936865c9b96dcdf3a42b263496d242889a6d1a63ec399c858a9abbf25ac93bf2192442442719b434b80b09eb4487938b1))
#print( modular_exp(0xa9e9df5b, 0xb9bca874, 0xb9bca875))

#p = gen_prime(32)
