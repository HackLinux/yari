/*

This random generator is the Mersenne Twister. The implementation is
borrowed from the public domain code at
http://www.qbrundage.com/michaelb/pubs/essays/random_number_generation.html
(Thanks Michael L. Brundage).

---

The Mersenne Twister is a new random number generator,
invented/discovered in 1996 by Matsumora and Nishimura. If you want to
know how it works.

MT is a twisted GFSR(624,397), similar in spirit to R250 and in my
tests it's comparable in speed to R250 (slightly slower than R250
alone, but faster than R250/521 combined). It takes up more space than
R250 or R521, but less than the two combined. MT has an amazing period
of 2^19937-1. Overall, I'd have to say that the Mersenne Twister is my
current favorite random number generator.

Legal crap

I hereby place this sofware in the public domain.
*/

#define MT_LEN       624

static int mt_index;
static unsigned long mt_buffer[MT_LEN] = {
    // Seed from http://www.random.org/integers/?num=1248&min=0&max=65535&col=16&base=16&format=plain&rnd=new
    0x0fc24d45, 0x9b71f430, 0xb339d43e, 0x14f65562, 0xf2ddfeb3,
    0xccabdd64, 0x928c9496, 0x8adb478c, 0x245c7314, 0xd5de234e,
    0x6c353a00, 0xa814ec84, 0x029dff91, 0xe54c4a09, 0x2a4ea351,
    0xc17c8d61, 0x2fa16bb3, 0xbede2d34, 0x3debf14b, 0x7cfaa781,
    0x2f0d9b1d, 0x9bc37429, 0x9de175c8, 0x3f080359, 0x4decd249,
    0x96775621, 0xdf6ba38e, 0xad265f5c, 0x74613c64, 0x96aa7a66,
    0xa68bfa95, 0xb4e3398c, 0x72df4e24, 0x4641ca4b, 0x127e4e67,
    0xc6b90f68, 0x5b8e9af9, 0x93575a44, 0x5bede177, 0x0bdaf684,
    0x6b2bf86d, 0x626d93c5, 0x5c81f60b, 0x6f4d2d8b, 0x22d8a3ff,
    0x11837ef3, 0x2fe8e35a, 0xd7a01173, 0x1b64191c, 0x36022d70,
    0xe619a9f3, 0xe1736341, 0x679ad9b0, 0xdabc2a7d, 0x990b73dd,
    0x66c31edf, 0x2da0f7fa, 0x5bf3b77c, 0x125ad6b6, 0x300d5b5a,
    0x0f93a9df, 0xce42ca7d, 0x05a7005d, 0xe5d95341, 0x84acd336,
    0xd4c48efc, 0x4559c4c7, 0xfbdc4712, 0xf3dd3222, 0xe68a8dfc,
    0x6dab7130, 0x026cdf51, 0x89f7d02c, 0x085a9679, 0x9ed57007,
    0x880b7040, 0xb8612e0c, 0xa30af970, 0xda41fa2e, 0x3a8fb120,
    0xf7292979, 0xec12f43e, 0x6fc85b72, 0x252e10f1, 0x7148a503,
    0x9a5a29b2, 0xb4f00c3c, 0xce08f985, 0x37cccbaf, 0x76389be9,
    0xea032dc0, 0x5d38c0bd, 0xc4ef9cd1, 0xe2a1905d, 0x7642bef8,
    0xa5811df9, 0x5995b363, 0x06fef1fb, 0xc2932c21, 0xb8f43ed5,
    0x1d3b4e61, 0x3f9fab0b, 0x37457ee6, 0x38b55a12, 0x5545b308,
    0xc8b2666a, 0xb1820104, 0xbe0fe5fc, 0x555ffaac, 0x72bfa87b,
    0xb0169eb3, 0xa01cd4c5, 0x33c7f3a5, 0xedbd843d, 0xc41acd89,
    0x019a8581, 0x3058d231, 0x79efac48, 0x1a017938, 0x0a4021a4,
    0x7ee40593, 0xdce60a22, 0x093956db, 0xbfbd0ff4, 0xb1a8ff41,
    0xd417513e, 0xbeac7913, 0x7e78cebb, 0x3319c578, 0x00ad7753,
    0x161490ef, 0xcd42d43c, 0x70f9991f, 0x8cc9e6a1, 0x6f3ff2d5,
    0xab7cf990, 0x944f74b1, 0xb54b5dd3, 0x6e9ba680, 0x26a86ff0,
    0x7552c7e8, 0xc0d1b285, 0x6ff7aa4e, 0x70a2b03e, 0x3da2e46f,
    0xcca9e07d, 0xa5b62e88, 0x3bd048d4, 0x95061ca7, 0x055ec6c6,
    0xc2aad096, 0xe46d52ae, 0xbe093e96, 0x8a7c1d9e, 0xfc544827,
    0x04506a79, 0x5bba48a7, 0xbac238e6, 0xbd3d684e, 0x66017a3f,
    0xd95a7eb1, 0x719b9e1f, 0x79b70bcd, 0x99b866a2, 0x8808c894,
    0x4322a807, 0x748a6080, 0xe23ff10a, 0x4b27af7c, 0xca695ee1,
    0x9bdac585, 0x13effa72, 0x0da9ea01, 0x52f43a9c, 0x5f63c48a,
    0x1fb59e44, 0x23427c2b, 0xc5ca3deb, 0xe63f081b, 0x10a0cbf6,
    0x3f9e65d8, 0xd94630f7, 0xd4794142, 0xae9cff2a, 0x6d37a736,
    0xb0e83388, 0xbf5e840b, 0x03f223d0, 0xb9994851, 0xa62f3a9b,
    0x1263e3d4, 0xfc1af911, 0xe976c741, 0x22cedf73, 0xcb2137ca,
    0xc508fa38, 0xa76c372f, 0xbf17084c, 0x77479f4b, 0xd193eed8,
    0x046bb78f, 0x79c7d7c9, 0xc30d57ed, 0x30f4da0e, 0xfaea8d7c,
    0x710c1bbf, 0xac48b05c, 0x13165b23, 0x451607bb, 0xac616418,
    0x639f428e, 0xfbfce873, 0xaa4422d9, 0x7cc1595a, 0x9dd77175,
    0x6480d204, 0xc872a942, 0x1d58df77, 0x3e44a2d5, 0x2966e535,
    0x7d46abca, 0x850e76e9, 0x2535eb1f, 0xa9ea0fea, 0x43dad6d2,
    0x94f1b8c0, 0xe8b1000b, 0xba7c8159, 0x9ce47143, 0xd417a43f,
    0xecb11ecc, 0xd9ccfa13, 0xed9222cd, 0xf323bec3, 0x8b1bbfc3,
    0x3d26da40, 0xa4af4931, 0xc344f123, 0xadb55604, 0x8343b148,
    0xfc8e8697, 0x8bfe51be, 0x48dac911, 0x2e18dcf9, 0xf98829a0,
    0x5a990583, 0xba7dce46, 0x2c95b8f3, 0xf4ce099f, 0xdb8f055e,
    0xafc1bf16, 0xf1ee58bb, 0xf37a9df3, 0xf5e81052, 0xe7d073f8,
    0xf10642ff, 0x8c1bda60, 0x9181cf36, 0x6f8e5a1e, 0xc1979934,
    0x75f14fd0, 0xefa3ab1f, 0x15809a88, 0x0873f770, 0xcb2b59f5,
    0x1631abcd, 0xa9619b73, 0xc70bd9bf, 0x22238ec3, 0x0892728f,
    0x3a4249ff, 0x20250c28, 0x74647f5f, 0xec540dd6, 0x5d56c90a,
    0x36827c40, 0x67e5cbfc, 0x51b17079, 0x1b5c7875, 0xeecee3d9,
    0xc41aa6ff, 0x9487eee5, 0x4a7dd5dd, 0xc679936f, 0xe0d35956,
    0x110c8ee4, 0x2e73d8fe, 0x4092b16f, 0xc163429a, 0x8c585b46,
    0xd2145e31, 0x11ff3efc, 0x5a9509c0, 0x4f2416bc, 0x07bcb294,
    0xc85ee4c0, 0x4ed44e3c, 0x9120f4fa, 0x5f694cfd, 0x690eeee3,
    0x9cc48c9f, 0x2efa4195, 0xb586b63d, 0xb0b933c5, 0x20804fb3,
    0x439e9264, 0x487ef7de, 0x8b8d580f, 0xb18aec43, 0x6c55eec4,
    0x5779f0ba, 0xc2871669, 0xa78a83b5, 0xc002dcab, 0xe9748f30,
    0x687786ad, 0xb8a29c88, 0x36c87212, 0xd850c4ae, 0x313b40c3,
    0xf94f9cee, 0x6750ae7c, 0xca8c143e, 0x808dba2f, 0x267e03de,
    0xd55c0580, 0x7f4d40d0, 0xea64d203, 0xbac25f9c, 0x50c480c9,
    0x3f2d6fa5, 0x9a50b652, 0x93ba6af8, 0x8b896613, 0x7b93235f,
    0x94902d33, 0x918a677c, 0x8b2b37f4, 0x101150f9, 0x3a08bbd2,
    0x3d19fb70, 0x435ca1c9, 0x8e8f3e54, 0xbf218359, 0x67c79bc3,
    0xd80c75b2, 0x962532b2, 0x975d99d8, 0xfc8a7adf, 0x4139bba8,
    0x39ba50ab, 0x3f2c5d7e, 0xa805d097, 0x3405cc89, 0x4db50e3a,
    0x97961a1e, 0xc6d2832c, 0xfe3d4cb3, 0xb8ddd5e0, 0x3887ca12,
    0xdc4b5cd1, 0x9f55fc88, 0x668f1c9e, 0xfa1aab72, 0x754fe1a8,
    0xf1f1faff, 0x1d13b77d, 0xc5e92399, 0x5ae0d504, 0xa9e702d0,
    0x31677ce6, 0xdebf5cdf, 0x27ef37c0, 0xa38d8622, 0xe116c01d,
    0x4476ee47, 0x6fb56927, 0x4ad30e1d, 0x8b2b015b, 0x144711a6,
    0x7aeb138e, 0xb951ad62, 0x01e53374, 0x5e8e5d18, 0xfd84373a,
    0x0d94ecf5, 0x6eb30e04, 0xe16daebb, 0xc44587d6, 0x15a59a92,
    0x4bce5d34, 0xaae85db3, 0x60291385, 0x2a31976f, 0xf904b44c,
    0xe8b07145, 0xd252f464, 0x9c32f30a, 0xe771e015, 0x77b1e75d,
    0xff41a3fc, 0xe64f0061, 0x3bc52249, 0x59ecec1a, 0xc8e0a108,
    0x9c36daca, 0x63e90750, 0x91c04d5f, 0x3247412f, 0xebe7cf0a,
    0xee90b0e2, 0x77012e5a, 0xd46871ca, 0x0111d13f, 0x69933759,
    0xd46c0546, 0xc17e6dea, 0x86d6380d, 0xd4cd83ed, 0x7ef14ae7,
    0xbc5324ce, 0xa367a6ee, 0x966ef172, 0x4d2feb0f, 0x794ed1cc,
    0x1b099b27, 0xa5738874, 0x7036ed6e, 0x018ad8f4, 0xbba1302e,
    0xab19e53e, 0xfec20700, 0xecb147fa, 0x3247835e, 0xb716715d,
    0x7b98ea02, 0x208fc9ad, 0xdcbbfa29, 0x040a5e3b, 0x8e751b70,
    0x32258e50, 0x9be910cb, 0x1425d065, 0xb58120c0, 0xc6e69688,
    0xf920157e, 0x31837709, 0x54dec272, 0x88221035, 0xcdff1473,
    0x39e8b5a2, 0xfeaac0f7, 0x348881c3, 0x621a5689, 0xfd8d388a,
    0x9b669e99, 0xbe351c07, 0x31e8d174, 0x11e3a359, 0x1ae472c6,
    0xfc7b3b92, 0x9ab12f0a, 0x6303446d, 0x3b764800, 0xe0fa16ca,
    0xd31c5963, 0xaa7212ef, 0x8ee70db0, 0x046e3c3a, 0x2bdcfca8,
    0x15655aab, 0x9f4ac783, 0xf371485e, 0xb4bc2250, 0x34ceae3d,
    0xbe712a07, 0x51678449, 0xea8d45b6, 0x7d52a5e3, 0xfca13126,
    0xdf133443, 0xdc196a20, 0x7ba32ba3, 0x815248e2, 0x00ee4054,
    0xd4e247a7, 0x2c32e340, 0x0fae8424, 0x65d3bdae, 0x89c31306,
    0xf7125ee4, 0xbc91c329, 0xb3efe150, 0x07271b32, 0xb04c704a,
    0xeca8f6e6, 0x43e998e4, 0x652e51b2, 0x2c70252e, 0x457922a7,
    0x6482db14, 0xe030bc88, 0xa072db13, 0x1e9ea883, 0x7571f909,
    0x8754d88f, 0xf08371d3, 0xdea93738, 0x1511f3c0, 0xf89b8c23,
    0xfdab1229, 0xa80b8dc4, 0x471f7a0b, 0xe3cffa8d, 0x29306358,
    0x3a33ac8e, 0x184b5943, 0x1d9721c0, 0xd53da2db, 0x017471b8,
    0xcb9af83e, 0x7de82792, 0xfb4984c5, 0x348ea81d, 0x697403a1,
    0x765b1483, 0x7309e116, 0x5b660ef9, 0x1c5197cf, 0xb6c9b909,
    0xf9ac20e9, 0x17387176, 0x97e26e7b, 0x9e8b1a67, 0x339a2da3,
    0xdc48e0e8, 0x34fcff14, 0xacb8e4c2, 0xeb23c3db, 0x89e15383,
    0x0d2b6ec9, 0x58cfca57, 0x897e7222, 0xcd42993b, 0xdd2f1051,
    0x21530810, 0x22e78c36, 0x8dab7703, 0xab564350, 0xdbc47480,
    0xaec6bc02, 0xc98f87f3, 0xcd74232c, 0x2f77b661, 0xb3b23357,
    0xfe40fbbc, 0x59a0bcd5, 0x649e696e, 0x26695a63, 0x7d2b11a2,
    0xe7b077b5, 0xf1152cc1, 0xd85b9f8d, 0x2001add3, 0x36139118,
    0x0aa89116, 0x3d738079, 0x6e4471dc, 0xef8e2561, 0x10bbd825,
    0x69e63f4c, 0x04f7e50f, 0xdd5e6f88, 0x047fbdab, 0xe24fb2ba,
    0xdc066311, 0xde5dd721, 0xff7b1fe0, 0x7ccea4a8, 0x4f336f93,
    0xc15e02f0, 0x3ff52d0f, 0xa7cb5d0d, 0x4a8e4ba9, 0x3d3c55f7,
    0x999ec913, 0xf7edaaa7, 0x6221a38e, 0xc0425883, 0x8d2fb2fa,
    0x99200a31, 0xdd8467e7, 0x539d8c89, 0x507a03ec, 0x5560293c,
    0xeb340cd3, 0x066a4b14, 0xa474cadf, 0x1d529432, 0xb327d771,
    0xa96b10eb, 0xc6757d29, 0x9450e8f4, 0x109f072a, 0x3b95c78c,
    0x43f63c34, 0x1690756b, 0x5d941ecc, 0xb192bfe9, 0xa21b9918,
    0xe6ccbad8, 0x53829dea, 0x5b5246b8, 0xfe0703f5, 0x3d36e6af,
    0x55e04f2b, 0x43fa17fd, 0xfe08cb3d, 0x3bf4c774, 0xadd481ec,
    0x112053c0, 0x3db3937d, 0xe5bed5b5, 0x42d39b87,


};

void srandom(unsigned seed)
{
    int i;
    for (i = 0; i < MT_LEN; i++)
        mt_buffer[i] ^= seed;
    mt_index = 0;
}

#define MT_IA           397
#define MT_IB           (MT_LEN - MT_IA)
#define UPPER_MASK      0x80000000
#define LOWER_MASK      0x7FFFFFFF
#define MATRIX_A        0x9908B0DF
#define TWIST(b,i,j)    ((b)[i] & UPPER_MASK) | ((b)[j] & LOWER_MASK)
#define MAGIC(s)        (((s)&1)*MATRIX_A)

unsigned long random(void)
{
    unsigned long * b = mt_buffer;
    int idx = mt_index;
    unsigned long s;
    int i;

    if (idx == MT_LEN*sizeof(unsigned long)) {
        idx = 0;
        i = 0;

        for (; i < MT_IB; i++) {
            s = TWIST(b, i, i+1);
            b[i] = b[i + MT_IA] ^ (s >> 1) ^ MAGIC(s);
        }

        for (; i < MT_LEN-1; i++) {
            s = TWIST(b, i, i+1);
            b[i] = b[i - MT_IB] ^ (s >> 1) ^ MAGIC(s);
        }

        s = TWIST(b, MT_LEN-1, 0);
        b[MT_LEN-1] = b[MT_IA-1] ^ (s >> 1) ^ MAGIC(s);
    }

    mt_index = idx + sizeof(unsigned long);

    return *(unsigned long *)((unsigned char *)b + idx);

    /*
    Matsumoto and Nishimura additionally confound the bits returned to the caller
    but this doesn't increase the randomness, and slows down the generator by
    as much as 25%.  So I omit these operations here.

    r ^= (r >> 11);
    r ^= (r << 7) & 0x9D2C5680;
    r ^= (r << 15) & 0xEFC60000;
    r ^= (r >> 18);
    */
}
