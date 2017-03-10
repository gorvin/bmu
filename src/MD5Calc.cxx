#include <bmu/MD5Calc.h>
#include <string.h>

/* Constants for MD5Transform routine. */
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

namespace beam_me_up{

/* MD5 context. */
struct MD5_CTX{
  unsigned int state[4];/* state (ABCD) */
  unsigned int count[2];/* number of bits, modulo 2^64 (lsb first) */
  unsigned char buffer[64];/* input buffer */
};
static void MD5Update (MD5_CTX*, unsigned char const*, unsigned int);

static void MD5Transform (unsigned int [4], unsigned char const [64]);
static void ultole (unsigned char*, unsigned int*, unsigned int);
static void letoul (unsigned int*, unsigned char const*, unsigned int);

static unsigned char PADDING[64] = {
  0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

unsigned int F(unsigned int x, unsigned int y, unsigned int z) { return (x&y)|((~x)&z); }
unsigned int G(unsigned int x, unsigned int y, unsigned int z) { return F(z,x,y); }
unsigned int H(unsigned int x, unsigned int y, unsigned int z) { return x^y^z; }
unsigned int I(unsigned int x, unsigned int y, unsigned int z) { return y^(x|(~z));}

/* ROTATE_LEFT rotates x left n bits. */
//#define ROTATE_LEFT(x, n) ( (x << n) | (x >> (32-n)) )

void md5round(unsigned int& a, unsigned int b, unsigned int x, unsigned int s, unsigned int ac, unsigned int md5fghi) {
    a += md5fghi + x + ac;
    a = (a<<s)|(a>>(32-s));//ROTATE_LEFT(a,s);
    a += b;
}

/* FF, GG, HH, and II transformations for rounds 1, 2, 3, and 4. */
void FF(unsigned int& a, unsigned int b, unsigned int c, unsigned int d, unsigned int x, unsigned int s, unsigned int ac)
    {return md5round(a,b,x,s,ac,F(b,c,d)); }
void GG(unsigned int& a, unsigned int b, unsigned int c, unsigned int d, unsigned int x, unsigned int s, unsigned int ac)
    {return md5round(a,b,x,s,ac,G(b,c,d)); }
void HH(unsigned int& a, unsigned int b, unsigned int c, unsigned int d, unsigned int x, unsigned int s, unsigned int ac)
    {return md5round(a,b,x,s,ac,H(b,c,d)); }
void II(unsigned int& a, unsigned int b, unsigned int c, unsigned int d, unsigned int x, unsigned int s, unsigned int ac)
    {return md5round(a,b,x,s,ac,I(b,c,d)); }



unsigned int const MD5Calc::state_init[4] = {0x67452301,0xefcdab89,0x98badcfe,0x10325476};

void MD5Calc::Finish(unsigned char digest[16])
{
    unsigned char bits[8];
    ultole (bits, count, 8);/* Save number of bits */

    /* Pad out to 56 mod 64. */
    unsigned int index = (unsigned int)((count[0] >> 3) & 0x3f);
    unsigned int padLen = (index < 56) ? (56 - index) : (120 - index);

    Update(PADDING, padLen);
    Update(bits, 8);/* Append length (before padding) */
    ultole(digest, state, 16);/* Store state in digest */
}

void MD5Calc::Update(unsigned char const* input, unsigned int inputLen)
{

    /* Compute number of bytes mod 64 */
    unsigned int index = (unsigned int)((count[0] >> 3) & 0x3F);

    /* Update number of bits */
    if ((count[0] += ((unsigned int)inputLen << 3)) < ((unsigned int)inputLen << 3))
        count[1]++;
    count[1] += ((unsigned int)inputLen >> 29);

    unsigned int partLen = 64 - index;

    unsigned int i;
    /* Transform as many times as possible. */
    if (inputLen >= partLen) {
      memcpy((unsigned char*)&buffer[index], (unsigned char*)input, partLen);
      MD5Transform (state, buffer);

      for (i = partLen; i + 63 < inputLen; i += 64) MD5Transform (state, &input[i]);

      index = 0;
    } else
      i = 0;

    /* Buffer remaining input */
    memcpy((unsigned char*)&buffer[index], (unsigned char*)&input[i], inputLen-i);
}

MD5Calc::MD5Calc(void)
{
    for(int i=0; i<4; i++) state[i] = state_init[i];
    count[0] = count[1] = 0;
}

MD5Calc::~MD5Calc(void)
{
    memset ((unsigned char*)&state, 0, 4*(4+2)+64);
}

void MD5Calculate (unsigned char digest[16], unsigned char const* input, unsigned int inputLen)
{
  unsigned char bits[8];
  unsigned int index, padLen;
  /* MD5 initialization. Begins an MD5 operation, writing a new context. */
  MD5_CTX context = {{0x67452301,0xefcdab89,0x98badcfe,0x10325476},{0,0},{}};


  MD5Update(&context, input, inputLen);//main calculation


  ultole (bits, context.count, 8);/* Save number of bits */

  /* Pad out to 56 mod 64. */
  index = (unsigned int)((context.count[0] >> 3) & 0x3f);
  padLen = (index < 56) ? (56 - index) : (120 - index);

  MD5Update (&context, PADDING, padLen);
  MD5Update (&context, bits, 8);/* Append length (before padding) */
  ultole (digest, context.state, 16);/* Store state in digest */

  /* Zeroize sensitive information. */
  memset ((unsigned char*)&context, 0, sizeof (context));
}

unsigned int MD5_32(unsigned char* input, unsigned int inputLen)
{
    unsigned char digest[16];
    MD5Calculate(digest, input, inputLen);

    unsigned int* x = (unsigned int*)&digest[0];
    return x[0]^x[1]^x[2]^x[3];
}

/* MD5 block update operation. Continues an MD5 message-digest operation,
 processing another message block, and updating the context.
 */
static void MD5Update (MD5_CTX* context, unsigned char const* input, unsigned int inputLen)
{
  unsigned int i;
  unsigned int index;
  unsigned int partLen;

  /* Compute number of bytes mod 64 */
  index = (unsigned int)((context->count[0] >> 3) & 0x3F);

  /* Update number of bits */
  if ((context->count[0] += ((unsigned int)inputLen << 3)) < ((unsigned int)inputLen << 3))
      context->count[1]++;
  context->count[1] += ((unsigned int)inputLen >> 29);

  partLen = 64 - index;

  /* Transform as many times as possible. */
  if (inputLen >= partLen) {
      memcpy((unsigned char*)&context->buffer[index], (unsigned char*)input, partLen);
      MD5Transform (context->state, context->buffer);

      for (i = partLen; i + 63 < inputLen; i += 64)
          MD5Transform (context->state, &input[i]);

      index = 0;
  } else
      i = 0;

  /* Buffer remaining input */
  memcpy((unsigned char*)&context->buffer[index], (unsigned char*)&input[i], inputLen-i);
}

/* MD5 basic transformation. Transforms state based on block. */
static void MD5Transform (unsigned int state[4], unsigned char const block[64])
{
  unsigned int a = state[0], b = state[1], c = state[2], d = state[3], x[16];

  letoul (x, block, 64);

  /* Round 1 */
  FF (a, b, c, d, x[ 0], S11, 0xd76aa478); /* 1 */
  FF (d, a, b, c, x[ 1], S12, 0xe8c7b756); /* 2 */
  FF (c, d, a, b, x[ 2], S13, 0x242070db); /* 3 */
  FF (b, c, d, a, x[ 3], S14, 0xc1bdceee); /* 4 */
  FF (a, b, c, d, x[ 4], S11, 0xf57c0faf); /* 5 */
  FF (d, a, b, c, x[ 5], S12, 0x4787c62a); /* 6 */
  FF (c, d, a, b, x[ 6], S13, 0xa8304613); /* 7 */
  FF (b, c, d, a, x[ 7], S14, 0xfd469501); /* 8 */
  FF (a, b, c, d, x[ 8], S11, 0x698098d8); /* 9 */
  FF (d, a, b, c, x[ 9], S12, 0x8b44f7af); /* 10 */
  FF (c, d, a, b, x[10], S13, 0xffff5bb1); /* 11 */
  FF (b, c, d, a, x[11], S14, 0x895cd7be); /* 12 */
  FF (a, b, c, d, x[12], S11, 0x6b901122); /* 13 */
  FF (d, a, b, c, x[13], S12, 0xfd987193); /* 14 */
  FF (c, d, a, b, x[14], S13, 0xa679438e); /* 15 */
  FF (b, c, d, a, x[15], S14, 0x49b40821); /* 16 */

 /* Round 2 */
  GG (a, b, c, d, x[ 1], S21, 0xf61e2562); /* 17 */
  GG (d, a, b, c, x[ 6], S22, 0xc040b340); /* 18 */
  GG (c, d, a, b, x[11], S23, 0x265e5a51); /* 19 */
  GG (b, c, d, a, x[ 0], S24, 0xe9b6c7aa); /* 20 */
  GG (a, b, c, d, x[ 5], S21, 0xd62f105d); /* 21 */
  GG (d, a, b, c, x[10], S22,  0x2441453); /* 22 */
  GG (c, d, a, b, x[15], S23, 0xd8a1e681); /* 23 */
  GG (b, c, d, a, x[ 4], S24, 0xe7d3fbc8); /* 24 */
  GG (a, b, c, d, x[ 9], S21, 0x21e1cde6); /* 25 */
  GG (d, a, b, c, x[14], S22, 0xc33707d6); /* 26 */
  GG (c, d, a, b, x[ 3], S23, 0xf4d50d87); /* 27 */
  GG (b, c, d, a, x[ 8], S24, 0x455a14ed); /* 28 */
  GG (a, b, c, d, x[13], S21, 0xa9e3e905); /* 29 */
  GG (d, a, b, c, x[ 2], S22, 0xfcefa3f8); /* 30 */
  GG (c, d, a, b, x[ 7], S23, 0x676f02d9); /* 31 */
  GG (b, c, d, a, x[12], S24, 0x8d2a4c8a); /* 32 */

  /* Round 3 */
  HH (a, b, c, d, x[ 5], S31, 0xfffa3942); /* 33 */
  HH (d, a, b, c, x[ 8], S32, 0x8771f681); /* 34 */
  HH (c, d, a, b, x[11], S33, 0x6d9d6122); /* 35 */
  HH (b, c, d, a, x[14], S34, 0xfde5380c); /* 36 */
  HH (a, b, c, d, x[ 1], S31, 0xa4beea44); /* 37 */
  HH (d, a, b, c, x[ 4], S32, 0x4bdecfa9); /* 38 */
  HH (c, d, a, b, x[ 7], S33, 0xf6bb4b60); /* 39 */
  HH (b, c, d, a, x[10], S34, 0xbebfbc70); /* 40 */
  HH (a, b, c, d, x[13], S31, 0x289b7ec6); /* 41 */
  HH (d, a, b, c, x[ 0], S32, 0xeaa127fa); /* 42 */
  HH (c, d, a, b, x[ 3], S33, 0xd4ef3085); /* 43 */
  HH (b, c, d, a, x[ 6], S34,  0x4881d05); /* 44 */
  HH (a, b, c, d, x[ 9], S31, 0xd9d4d039); /* 45 */
  HH (d, a, b, c, x[12], S32, 0xe6db99e5); /* 46 */
  HH (c, d, a, b, x[15], S33, 0x1fa27cf8); /* 47 */
  HH (b, c, d, a, x[ 2], S34, 0xc4ac5665); /* 48 */

  /* Round 4 */
  II (a, b, c, d, x[ 0], S41, 0xf4292244); /* 49 */
  II (d, a, b, c, x[ 7], S42, 0x432aff97); /* 50 */
  II (c, d, a, b, x[14], S43, 0xab9423a7); /* 51 */
  II (b, c, d, a, x[ 5], S44, 0xfc93a039); /* 52 */
  II (a, b, c, d, x[12], S41, 0x655b59c3); /* 53 */
  II (d, a, b, c, x[ 3], S42, 0x8f0ccc92); /* 54 */
  II (c, d, a, b, x[10], S43, 0xffeff47d); /* 55 */
  II (b, c, d, a, x[ 1], S44, 0x85845dd1); /* 56 */
  II (a, b, c, d, x[ 8], S41, 0x6fa87e4f); /* 57 */
  II (d, a, b, c, x[15], S42, 0xfe2ce6e0); /* 58 */
  II (c, d, a, b, x[ 6], S43, 0xa3014314); /* 59 */
  II (b, c, d, a, x[13], S44, 0x4e0811a1); /* 60 */
  II (a, b, c, d, x[ 4], S41, 0xf7537e82); /* 61 */
  II (d, a, b, c, x[11], S42, 0xbd3af235); /* 62 */
  II (c, d, a, b, x[ 2], S43, 0x2ad7d2bb); /* 63 */
  II (b, c, d, a, x[ 9], S44, 0xeb86d391); /* 64 */

  state[0] += a;
  state[1] += b;
  state[2] += c;
  state[3] += d;

  /* Zeroize sensitive information.*/
  memset ((unsigned char*)x, 0, sizeof (x));
}

/* ultoles input (unsigned int) into output (unsigned char). Assumes len is a multiple of 4. */
static void ultole (unsigned char* output, unsigned int* input, unsigned int len)
{
  unsigned int i, j;

  for (i = 0, j = 0; j < len; i++, j += 4) {
    output[j] = (unsigned char)(input[i] & 0xff);
    output[j+1] = (unsigned char)((input[i] >> 8) & 0xff);
    output[j+2] = (unsigned char)((input[i] >> 16) & 0xff);
    output[j+3] = (unsigned char)((input[i] >> 24) & 0xff);
  }
}

/* letouls input (unsigned char) into output (unsigned int). Assumes len is
  a multiple of 4.
 */
static void letoul (unsigned int* output, unsigned char const* input, unsigned int len)
{
  unsigned int i, j;

  for (i = 0, j = 0; j < len; i++, j += 4)
    output[i] = ((unsigned int)input[j]) | (((unsigned int)input[j+1]) << 8) |
     (((unsigned int)input[j+2]) << 16) | (((unsigned int)input[j+3]) << 24);
}

}
