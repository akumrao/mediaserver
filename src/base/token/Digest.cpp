

#include "Digest.h"

 
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <string.h>

#define S11 7
#define S12 12
#define S13 17
#define S14 22
#define S21 5
#define S22 9
#define S23 14
#define S24 20
#define S31 4
#define S32 11
#define S33 16
#define S34 23
#define S41 6
#define S42 10
#define S43 15
#define S44 21

/* F, G, H and I are basic MD5 functions.
 */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTL rotates x left n bits.
 */
#define ROTL(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4.
   Rotation is separate from addition to prevent recomputation.
 */
#define FF(a, b, c, d, x, s, ac) { \
	(a) += F ((b), (c), (d)) + (x) + (uint32_t)(ac); \
	(a) = ROTL ((a), (s)); \
	(a) += (b); \
}

#define GG(a, b, c, d, x, s, ac) { \
	(a) += G ((b), (c), (d)) + (x) + (uint32_t)(ac); \
	(a) = ROTL ((a), (s)); \
	(a) += (b); \
}

#define HH(a, b, c, d, x, s, ac) { \
	(a) += H ((b), (c), (d)) + (x) + (uint32_t)(ac); \
	(a) = ROTL ((a), (s)); \
	(a) += (b); \
}

#define II(a, b, c, d, x, s, ac) { \
	(a) += I ((b), (c), (d)) + (x) + (uint32_t)(ac); \
	(a) = ROTL ((a), (s)); \
	(a) += (b); \
}

//
// SHA1 transformations
//
#define BLK0(_A, _I) (_A[_I] = (ROTL(_A[_I], 24) & 0xff00ff00) | (ROTL(_A[_I], 8) & 0x00ff00ff))
#define BLK(_A, _I) (_A[_I & 0x0f] = ROTL(_A[(_I + 13) & 0x0f] ^\
			_A[(_I + 8) & 0x0f] ^ _A[(_I + 2) & 0x0f] ^ _A[_I & 0x0f], 1))

#define R0(v,w,x,y,z,_A,_I) {\
	z += ((w & (x ^ y)) ^ y) + BLK0(_A, _I) + 0x5a827999 + ROTL(v, 5);\
	w = ROTL(w, 30);\
}

#define R1(v,w,x,y,z,_A,_I) {\
	z += ((w & (x ^ y)) ^ y) + BLK(_A, _I) + 0x5a827999 + ROTL(v, 5);\
	w = ROTL(w, 30);\
}

#define R2(v,w,x,y,z,_A,_I) {\
	z += (w ^ x ^ y) + BLK(_A, _I) + 0x6ed9eba1 + ROTL(v, 5);\
	w = ROTL(w, 30);\
}

#define R3(v,w,x,y,z,_A,_I) {\
	z += (((w | x) & y) | (w & x)) + BLK(_A, _I) + 0x8f1bbcdc + ROTL(v, 5);\
	w = ROTL(w, 30);\
}

#define R4(v,w,x,y,z,_A,_I) {\
	z += (w ^ x ^ y) + BLK(_A, _I) + 0xca62c1d6 + ROTL(v, 5);\
	w = ROTL(w, 30);\
}
                            /*### @-prefix ************************************/
                            /*### @+DigestMD5::PADDING ************************/
const uint8_t DigestMD5::PADDING[64] =
{
	0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};
                            /*### @-DigestMD5::PADDING ************************/
                            /*### @+DigestMD5::DigestMD5 {{{2 *****************/
DigestMD5::DigestMD5()
{
	init();
}
                            /*### @-DigestMD5::DigestMD5 }}}2 *****************/
                            /*### @+DigestMD5::~DigestMD5 {{{2 ****************/
DigestMD5::~DigestMD5()
{
}
                            /*### @-DigestMD5::~DigestMD5 }}}2 ****************/
                            /*### @+DigestMD5::final {{{2 *********************/
void DigestMD5::final(
	uint8_t digest[MD5_DIGESTSIZE])
{
	uint8_t bits[8];
	uint32_t idx, padLen;

	/* Save number of bits */
	u32to8(bits, mCount, 8);

	/* Pad out to 56 mod 64. */
	idx = static_cast<uint32_t>((mCount[0] >> 3) & 0x3f);
	padLen = (idx < 56) ? (56 - idx) : (120 - idx);
	update(PADDING, padLen);

	/* Append length (before padding) */
	update(bits, 8);

	/* Store mState in digest */
	u32to8(digest, mState, 16);
}
                            /*### @-DigestMD5::final }}}2 *********************/
                            /*### @+DigestMD5::init {{{2 **********************/
DigestMD5& DigestMD5::init()
{
	mCount[0] = mCount[1] = 0;

	/* Load magic initialization constants. */
	mState[0] = 0x67452301;
	mState[1] = 0xefcdab89;
	mState[2] = 0x98badcfe;
	mState[3] = 0x10325476;

	return *this;
}
                            /*### @-DigestMD5::init }}}2 **********************/
                            /*### @+DigestMD5::update.std {{{2 ****************/
DigestMD5& DigestMD5::update(
	const void* inbuf,
	uint32_t len)
{
	const uint8_t* input = reinterpret_cast<const uint8_t*>(inbuf);
	uint32_t i, idx, partLen;

	/* Compute number of bytes mod 64 */
	idx = ((mCount[0] >> 3) & 0x3f);

	/* Update number of bits */
	if ( (mCount[0] += (len << 3)) < (len << 3) )
		mCount[1]++;
	mCount[1] += len >> 29;

	partLen = 64 - idx;

	/* Transform as many times as possible. */
	if ( len >= partLen ) {
		memcpy(mBuffer + idx, input, partLen);
		xform(mBuffer);

		for (i = partLen; i + 63 < len; i += 64)
			xform(&input[i]);

		idx = 0;
	}
	else
		i = 0;

	/* Buffer remaining input */
	memcpy(mBuffer + idx, input + i, len - i);

	return *this;
}
                            /*### @-DigestMD5::update.std }}}2 ****************/
                            /*### @+DigestMD5::update.file {{{2 ***************/

/*
bool DigestMD5::update(
	String& emsg,
	const String& fname,
	off_t offset)
{
	int fd = open(fname.c_str(), O_RDONLY);
	if ( fd == -1 ) {
		emsg.assignf("open('%s'): %s", fname.c_str(), strerror(errno));
		return false;
	}

	if ( lseek(fd, offset, SEEK_SET) == -1 ) {
		emsg.assignf("seek('%s', %llu): %s", fname.c_str(), (uint64_t)offset, strerror(errno));
		close(fd);
		return false;
	}

	uint8_t buf[4096];
	ssize_t n;
	while ( (n = read(fd, buf, sizeof(buf))) > 0 )
		update(buf, n);

	close(fd);

	if ( n == -1 ) {
		emsg.assignf("read('%s'): %s", fname.c_str(), strerror(errno));
		return false;
	}
	else
		return true;
}
 */
                            /*### @-DigestMD5::update.file }}}2 ***************/
                            /*### @+DigestMD5::u32to8 {{{2 ********************/
void DigestMD5::u32to8(
	uint8_t* out,
	const uint32_t* in,
	uint32_t nbytes)
{
	uint32_t i, j, k;

	for ( i = 0, j = 0; j < nbytes; i++, j += 4 ) {
		k = in[i];
		out[j] = static_cast<uint8_t>(k & 0xff);
		out[j+1] = static_cast<uint8_t>((k >> 8) & 0xff);
		out[j+2] = static_cast<uint8_t>((k >> 16) & 0xff);
		out[j+3] = static_cast<uint8_t>((k >> 24) & 0xff);
	}
}
                            /*### @-DigestMD5::u32to8 }}}2 ********************/
                            /*### @+DigestMD5::u8to32 {{{2 ********************/
void DigestMD5::u8to32(
	uint32_t* out,
	const uint8_t* in,
	uint32_t nbytes)
{
	uint32_t i, j;

	for ( i = 0, j = 0; j < nbytes; i++, j += 4 ) {
		out[i] = (static_cast<uint32_t>(in[j]))
				| ((static_cast<uint32_t>(in[j + 1])) << 8)
				| ((static_cast<uint32_t>(in[j + 2])) << 16)
				| ((static_cast<uint32_t>(in[j + 3])) << 24);
	}
}
                            /*### @-DigestMD5::u8to32 }}}2 ********************/
                            /*### @+DigestMD5::xform {{{2 *********************/
void DigestMD5::xform(
	const uint8_t buf[64])
{
	uint32_t a = mState[0], b = mState[1], c = mState[2], d = mState[3], x[16];

	u8to32(x, buf, 64);

	/* Round 1 */
	FF(a, b, c, d, x[0], S11, 0xd76aa478);	/* 1 */
	FF(d, a, b, c, x[1], S12, 0xe8c7b756);	/* 2 */
	FF(c, d, a, b, x[2], S13, 0x242070db);	/* 3 */
	FF(b, c, d, a, x[3], S14, 0xc1bdceee);	/* 4 */
	FF(a, b, c, d, x[4], S11, 0xf57c0faf);	/* 5 */
	FF(d, a, b, c, x[5], S12, 0x4787c62a);	/* 6 */
	FF(c, d, a, b, x[6], S13, 0xa8304613);	/* 7 */
	FF(b, c, d, a, x[7], S14, 0xfd469501);	/* 8 */
	FF(a, b, c, d, x[8], S11, 0x698098d8);	/* 9 */
	FF(d, a, b, c, x[9], S12, 0x8b44f7af);	/* 10 */
	FF(c, d, a, b, x[10], S13, 0xffff5bb1);	/* 11 */
	FF(b, c, d, a, x[11], S14, 0x895cd7be);	/* 12 */
	FF(a, b, c, d, x[12], S11, 0x6b901122);	/* 13 */
	FF(d, a, b, c, x[13], S12, 0xfd987193);	/* 14 */
	FF(c, d, a, b, x[14], S13, 0xa679438e);	/* 15 */
	FF(b, c, d, a, x[15], S14, 0x49b40821);	/* 16 */

	/* Round 2 */
	GG(a, b, c, d, x[1], S21, 0xf61e2562);	/* 17 */
	GG(d, a, b, c, x[6], S22, 0xc040b340);	/* 18 */
	GG(c, d, a, b, x[11], S23, 0x265e5a51);	/* 19 */
	GG(b, c, d, a, x[0], S24, 0xe9b6c7aa);	/* 20 */
	GG(a, b, c, d, x[5], S21, 0xd62f105d);	/* 21 */
	GG(d, a, b, c, x[10], S22, 0x2441453);	/* 22 */
	GG(c, d, a, b, x[15], S23, 0xd8a1e681);	/* 23 */
	GG(b, c, d, a, x[4], S24, 0xe7d3fbc8);	/* 24 */
	GG(a, b, c, d, x[9], S21, 0x21e1cde6);	/* 25 */
	GG(d, a, b, c, x[14], S22, 0xc33707d6);	/* 26 */
	GG(c, d, a, b, x[3], S23, 0xf4d50d87);	/* 27 */
	GG(b, c, d, a, x[8], S24, 0x455a14ed);	/* 28 */
	GG(a, b, c, d, x[13], S21, 0xa9e3e905);	/* 29 */
	GG(d, a, b, c, x[2], S22, 0xfcefa3f8);	/* 30 */
	GG(c, d, a, b, x[7], S23, 0x676f02d9);	/* 31 */
	GG(b, c, d, a, x[12], S24, 0x8d2a4c8a);	/* 32 */

	/* Round 3 */
	HH(a, b, c, d, x[5], S31, 0xfffa3942);	/* 33 */
	HH(d, a, b, c, x[8], S32, 0x8771f681);	/* 34 */
	HH(c, d, a, b, x[11], S33, 0x6d9d6122);	/* 35 */
	HH(b, c, d, a, x[14], S34, 0xfde5380c);	/* 36 */
	HH(a, b, c, d, x[1], S31, 0xa4beea44);	/* 37 */
	HH(d, a, b, c, x[4], S32, 0x4bdecfa9);	/* 38 */
	HH(c, d, a, b, x[7], S33, 0xf6bb4b60);	/* 39 */
	HH(b, c, d, a, x[10], S34, 0xbebfbc70);	/* 40 */
	HH(a, b, c, d, x[13], S31, 0x289b7ec6);	/* 41 */
	HH(d, a, b, c, x[0], S32, 0xeaa127fa);	/* 42 */
	HH(c, d, a, b, x[3], S33, 0xd4ef3085);	/* 43 */
	HH(b, c, d, a, x[6], S34, 0x4881d05);	/* 44 */
	HH(a, b, c, d, x[9], S31, 0xd9d4d039);	/* 45 */
	HH(d, a, b, c, x[12], S32, 0xe6db99e5);	/* 46 */
	HH(c, d, a, b, x[15], S33, 0x1fa27cf8);	/* 47 */
	HH(b, c, d, a, x[2], S34, 0xc4ac5665);	/* 48 */

	/* Round 4 */
	II(a, b, c, d, x[0], S41, 0xf4292244);	/* 49 */
	II(d, a, b, c, x[7], S42, 0x432aff97);	/* 50 */
	II(c, d, a, b, x[14], S43, 0xab9423a7);	/* 51 */
	II(b, c, d, a, x[5], S44, 0xfc93a039);	/* 52 */
	II(a, b, c, d, x[12], S41, 0x655b59c3);	/* 53 */
	II(d, a, b, c, x[3], S42, 0x8f0ccc92);	/* 54 */
	II(c, d, a, b, x[10], S43, 0xffeff47d);	/* 55 */
	II(b, c, d, a, x[1], S44, 0x85845dd1);	/* 56 */
	II(a, b, c, d, x[8], S41, 0x6fa87e4f);	/* 57 */
	II(d, a, b, c, x[15], S42, 0xfe2ce6e0);	/* 58 */
	II(c, d, a, b, x[6], S43, 0xa3014314);	/* 59 */
	II(b, c, d, a, x[13], S44, 0x4e0811a1);	/* 60 */
	II(a, b, c, d, x[4], S41, 0xf7537e82);	/* 61 */
	II(d, a, b, c, x[11], S42, 0xbd3af235);	/* 62 */
	II(c, d, a, b, x[2], S43, 0x2ad7d2bb);	/* 63 */
	II(b, c, d, a, x[9], S44, 0xeb86d391);	/* 64 */

	mState[0] += a;
	mState[1] += b;
	mState[2] += c;
	mState[3] += d;
}
                            /*### @-DigestMD5::xform }}}2 *********************/
                            /*### @+DigestSHA1::DigestSHA1 {{{2 ***************/
DigestSHA1::DigestSHA1()
{
}
                            /*### @-DigestSHA1::DigestSHA1 }}}2 ***************/
                            /*### @+DigestSHA1::~DigestSHA1 {{{2 **************/
DigestSHA1::~DigestSHA1()
{
}
                            /*### @-DigestSHA1::~DigestSHA1 }}}2 **************/
                            /*### @+DigestSHA1::final {{{2 ********************/
void DigestSHA1::final(
	uint8_t digest[SHA1_DIGESTSIZE])
{
	uint8_t finalCount[8];

	for ( uint32_t i = 0; i < 8; ++i )
		finalCount[i] = (uint8_t)((mCount[(i >= 4) ? 0 : 1] >> ((3 - (i & 3)) * 8)) & 0xff);

	update("\200", 1);
	while ( (mCount[0] & 0x01f8) != 0x01c0 )
		update("\0", 1);

	update(finalCount, 8);

	for ( uint32_t i = 0; i < 20; ++i )
		digest[i] = (uint8_t)((mState[i >> 2] >> ((3 - (i & 3)) * 8)) & 0xff);
}
                            /*### @-DigestSHA1::final }}}2 ********************/
                            /*### @+DigestSHA1::init {{{2 *********************/
DigestSHA1& DigestSHA1::init()
{
	mCount[0] = mCount[1] = 0;

	mState[0] = 0x67452301;
	mState[1] = 0xefcdab89;
	mState[2] = 0x98badcfe;
	mState[3] = 0x10325476;
	mState[4] = 0xc3d2e1f0;

	memset(mBuffer, 0, sizeof(mBuffer));

	return *this;
}
                            /*### @-DigestSHA1::init }}}2 *********************/
                            /*### @+DigestSHA1::update.std {{{2 ***************/
DigestSHA1& DigestSHA1::update(
	const void* ibuf,
	uint32_t len)
{
	const uint8_t* buf = (const uint8_t*)ibuf;

	uint32_t j = (mCount[0] >> 3) & 63;
	if ( (mCount[0] += (len << 3)) < (len << 3) )
		mCount[1] += 1;
	mCount[1] += (len >> 29);

	uint32_t i = 0;
	if ( (j + len) > 63 ) {
		i = 64 - j;

		memcpy(mBuffer + j, buf, i);
		xform(mBuffer);

		for ( ; i + 63 < len; i += 64 )
			xform(buf + i);

		j = 0;
	}

	if ( i < len )
		memcpy(mBuffer + j, buf + i, len - i);

	return *this;
}
                            /*### @-DigestSHA1::update.std }}}2 ***************/
                            /*### @+DigestSHA1::update.file {{{2 **************/

/*
bool DigestSHA1::update(
	String& emsg,
	const String& fname,
	off_t offset)
{
	int fd = open(fname.c_str(), O_RDONLY);
	if ( fd == -1 ) {
		emsg.assignf("open('%s'): %s", fname.c_str(), strerror(errno));
		return false;
	}

	if ( lseek(fd, offset, SEEK_SET) == -1 ) {
		emsg.assignf("seek('%s', %llu): %s", fname.c_str(), (uint64_t)offset, strerror(errno));
		close(fd);
		return false;
	}

	uint8_t buf[4096];
	ssize_t n;
	while ( (n = read(fd, buf, sizeof(buf))) > 0 )
		update(buf, n);

	close(fd);

	if ( n == -1 ) {
		emsg.assignf("read('%s'): %s", fname.c_str(), strerror(errno));
		return false;
	}
	else
		return true;
}
*/
                            /*### @-DigestSHA1::update.file }}}2 **************/
                            /*### @+DigestSHA1::xform {{{2 ********************/
void DigestSHA1::xform(
	const uint8_t buf[64])
{
	uint32_t a, b, c, d, e;
	uint32_t l[16];

	a = mState[0];
	b = mState[1];
	c = mState[2];
	d = mState[3];
	e = mState[4];

	memcpy(l, buf, sizeof(l));

	R0(a,b,c,d,e,l, 0); R0(e,a,b,c,d,l, 1); R0(d,e,a,b,c,l, 2); R0(c,d,e,a,b,l, 3); R0(b,c,d,e,a,l, 4);
	R0(a,b,c,d,e,l, 5); R0(e,a,b,c,d,l, 6); R0(d,e,a,b,c,l, 7); R0(c,d,e,a,b,l, 8); R0(b,c,d,e,a,l, 9);
	R0(a,b,c,d,e,l,10); R0(e,a,b,c,d,l,11); R0(d,e,a,b,c,l,12); R0(c,d,e,a,b,l,13); R0(b,c,d,e,a,l,14);
	R0(a,b,c,d,e,l,15); R1(e,a,b,c,d,l,16); R1(d,e,a,b,c,l,17); R1(c,d,e,a,b,l,18); R1(b,c,d,e,a,l,19);

	R2(a,b,c,d,e,l,20); R2(e,a,b,c,d,l,21); R2(d,e,a,b,c,l,22); R2(c,d,e,a,b,l,23); R2(b,c,d,e,a,l,24);
	R2(a,b,c,d,e,l,25); R2(e,a,b,c,d,l,26); R2(d,e,a,b,c,l,27); R2(c,d,e,a,b,l,28); R2(b,c,d,e,a,l,29);
	R2(a,b,c,d,e,l,30); R2(e,a,b,c,d,l,31); R2(d,e,a,b,c,l,32); R2(c,d,e,a,b,l,33); R2(b,c,d,e,a,l,34);
	R2(a,b,c,d,e,l,35); R2(e,a,b,c,d,l,36); R2(d,e,a,b,c,l,37); R2(c,d,e,a,b,l,38); R2(b,c,d,e,a,l,39);

	R3(a,b,c,d,e,l,40); R3(e,a,b,c,d,l,41); R3(d,e,a,b,c,l,42); R3(c,d,e,a,b,l,43); R3(b,c,d,e,a,l,44);
	R3(a,b,c,d,e,l,45); R3(e,a,b,c,d,l,46); R3(d,e,a,b,c,l,47); R3(c,d,e,a,b,l,48); R3(b,c,d,e,a,l,49);
	R3(a,b,c,d,e,l,50); R3(e,a,b,c,d,l,51); R3(d,e,a,b,c,l,52); R3(c,d,e,a,b,l,53); R3(b,c,d,e,a,l,54);
	R3(a,b,c,d,e,l,55); R3(e,a,b,c,d,l,56); R3(d,e,a,b,c,l,57); R3(c,d,e,a,b,l,58); R3(b,c,d,e,a,l,59);

	R4(a,b,c,d,e,l,60); R4(e,a,b,c,d,l,61); R4(d,e,a,b,c,l,62); R4(c,d,e,a,b,l,63); R4(b,c,d,e,a,l,64);
	R4(a,b,c,d,e,l,65); R4(e,a,b,c,d,l,66); R4(d,e,a,b,c,l,67); R4(c,d,e,a,b,l,68); R4(b,c,d,e,a,l,69);
	R4(a,b,c,d,e,l,70); R4(e,a,b,c,d,l,71); R4(d,e,a,b,c,l,72); R4(c,d,e,a,b,l,73); R4(b,c,d,e,a,l,74);
	R4(a,b,c,d,e,l,75); R4(e,a,b,c,d,l,76); R4(d,e,a,b,c,l,77); R4(c,d,e,a,b,l,78); R4(b,c,d,e,a,l,79);

	mState[0] += a;
	mState[1] += b;
	mState[2] += c;
	mState[3] += d;
	mState[4] += e;
}
                            /*### @-DigestSHA1::xform }}}2 ********************/


/* ==================== editors ====================== */

