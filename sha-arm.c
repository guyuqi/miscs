#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <arm_neon.h>
#include <sys/auxv.h>
#include <asm/hwcap.h>
#include <arm_acle.h>
#include <sys/time.h>

#define ENABLE_ARM 1

long int GetTickCount() {
        struct timeval tv;
        gettimeofday(&tv, NULL);
        return tv.tv_sec * 1000000 + tv.tv_usec;
}


typedef struct {
    uint32_t state[5];
    uint32_t count[2];
    unsigned char buffer[64];
} SHA1_CTX;

 
#define rol(value, bits) (((value) << (bits)) | ((value) >> (32 - (bits))))

/* blk0() and blk() perform the initial expand. */
/* I got the idea of expanding during the round function from SSLeay */
#if BYTE_ORDER == LITTLE_ENDIAN
#define blk0(i) (block->l[i] = (rol(block->l[i],24)&0xFF00FF00) \
    |(rol(block->l[i],8)&0x00FF00FF))
#elif BYTE_ORDER == BIG_ENDIAN
#define blk0(i) block->l[i]
#else
#error "Endianness not defined!"
#endif
#define blk(i) (block->l[i&15] = rol(block->l[(i+13)&15]^block->l[(i+8)&15] \
    ^block->l[(i+2)&15]^block->l[i&15],1))

/* (R0+R1), R2, R3, R4 are the different operations used in SHA1 */
#define R0(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk0(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R1(v,w,x,y,z,i) z+=((w&(x^y))^y)+blk(i)+0x5A827999+rol(v,5);w=rol(w,30);
#define R2(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0x6ED9EBA1+rol(v,5);w=rol(w,30);
#define R3(v,w,x,y,z,i) z+=(((w|x)&y)|(w&x))+blk(i)+0x8F1BBCDC+rol(v,5);w=rol(w,30);
#define R4(v,w,x,y,z,i) z+=(w^x^y)+blk(i)+0xCA62C1D6+rol(v,5);w=rol(w,30);


void SHA1Transform(uint32_t state[5], const unsigned char buffer[64])
{
    uint32_t a, b, c, d, e;
    typedef union {
        unsigned char c[64];
        uint32_t l[16];
    } CHAR64LONG16;
#ifdef SHA1HANDSOFF
    CHAR64LONG16 block[1];  /* use array to appear as a pointer */
    memcpy(block, buffer, 64);
#else
    /* The following had better never be used because it causes the
     * pointer-to-const buffer to be cast into a pointer to non-const.
     * And the result is written through.  I threw a "const" in, hoping
     * this will cause a diagnostic.
     */
    //CHAR64LONG16* block = (const CHAR64LONG16*)buffer;
    CHAR64LONG16* block = (CHAR64LONG16*)buffer;
#endif
    /* Copy context->state[] to working vars */
    a = state[0];
    b = state[1];
    c = state[2];
    d = state[3];
    e = state[4];
    /* 4 rounds of 20 operations each. Loop unrolled. */
    R0(a,b,c,d,e, 0); R0(e,a,b,c,d, 1); R0(d,e,a,b,c, 2); R0(c,d,e,a,b, 3);
    R0(b,c,d,e,a, 4); R0(a,b,c,d,e, 5); R0(e,a,b,c,d, 6); R0(d,e,a,b,c, 7);
    R0(c,d,e,a,b, 8); R0(b,c,d,e,a, 9); R0(a,b,c,d,e,10); R0(e,a,b,c,d,11);
    R0(d,e,a,b,c,12); R0(c,d,e,a,b,13); R0(b,c,d,e,a,14); R0(a,b,c,d,e,15);
    R1(e,a,b,c,d,16); R1(d,e,a,b,c,17); R1(c,d,e,a,b,18); R1(b,c,d,e,a,19);
    R2(a,b,c,d,e,20); R2(e,a,b,c,d,21); R2(d,e,a,b,c,22); R2(c,d,e,a,b,23);
    R2(b,c,d,e,a,24); R2(a,b,c,d,e,25); R2(e,a,b,c,d,26); R2(d,e,a,b,c,27);
    R2(c,d,e,a,b,28); R2(b,c,d,e,a,29); R2(a,b,c,d,e,30); R2(e,a,b,c,d,31);
    R2(d,e,a,b,c,32); R2(c,d,e,a,b,33); R2(b,c,d,e,a,34); R2(a,b,c,d,e,35);
    R2(e,a,b,c,d,36); R2(d,e,a,b,c,37); R2(c,d,e,a,b,38); R2(b,c,d,e,a,39);
    R3(a,b,c,d,e,40); R3(e,a,b,c,d,41); R3(d,e,a,b,c,42); R3(c,d,e,a,b,43);
    R3(b,c,d,e,a,44); R3(a,b,c,d,e,45); R3(e,a,b,c,d,46); R3(d,e,a,b,c,47);
    R3(c,d,e,a,b,48); R3(b,c,d,e,a,49); R3(a,b,c,d,e,50); R3(e,a,b,c,d,51);
    R3(d,e,a,b,c,52); R3(c,d,e,a,b,53); R3(b,c,d,e,a,54); R3(a,b,c,d,e,55);
    R3(e,a,b,c,d,56); R3(d,e,a,b,c,57); R3(c,d,e,a,b,58); R3(b,c,d,e,a,59);
    R4(a,b,c,d,e,60); R4(e,a,b,c,d,61); R4(d,e,a,b,c,62); R4(c,d,e,a,b,63);
    R4(b,c,d,e,a,64); R4(a,b,c,d,e,65); R4(e,a,b,c,d,66); R4(d,e,a,b,c,67);
    R4(c,d,e,a,b,68); R4(b,c,d,e,a,69); R4(a,b,c,d,e,70); R4(e,a,b,c,d,71);
    R4(d,e,a,b,c,72); R4(c,d,e,a,b,73); R4(b,c,d,e,a,74); R4(a,b,c,d,e,75);
    R4(e,a,b,c,d,76); R4(d,e,a,b,c,77); R4(c,d,e,a,b,78); R4(b,c,d,e,a,79);
    /* Add the working vars back into context.state[] */
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
    state[4] += e;
    /* Wipe variables */
    a = b = c = d = e = 0;
#ifdef SHA1HANDSOFF
    memset(block, '\0', sizeof(block));
#endif
}

void SHA1Transform_arm64(uint32_t state[5], const unsigned char buffer[64])
{
        uint32x4_t ABCD, ABCD_SAVED;
        uint32x4_t TMP0, TMP1;
        uint32x4_t MSG0, MSG1, MSG2, MSG3;
        uint32_t   E0, E0_SAVED, E1;

        /* Load state */
        ABCD = vld1q_u32(&state[0]);
        E0 = state[4];

        /* Save state */
        ABCD_SAVED = ABCD;
        E0_SAVED = E0;

        /* Load message */
        MSG0 = vld1q_u32((const uint32_t*)(buffer));
        MSG1 = vld1q_u32((const uint32_t*)(buffer + 16));
        MSG2 = vld1q_u32((const uint32_t*)(buffer + 32));
        MSG3 = vld1q_u32((const uint32_t*)(buffer + 48));

        /* Reverse for little endian */
        MSG0 = vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(MSG0)));
        MSG1 = vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(MSG1)));
        MSG2 = vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(MSG2)));
        MSG3 = vreinterpretq_u32_u8(vrev32q_u8(vreinterpretq_u8_u32(MSG3)));

        TMP0 = vaddq_u32(MSG0, vdupq_n_u32(0x5A827999));
        TMP1 = vaddq_u32(MSG1, vdupq_n_u32(0x5A827999));

        /* Rounds 0-3 */
        E1 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
        ABCD = vsha1cq_u32(ABCD, E0, TMP0);
        TMP0 = vaddq_u32(MSG2, vdupq_n_u32(0x5A827999));
        MSG0 = vsha1su0q_u32(MSG0, MSG1, MSG2);

        /* Rounds 4-7 */
        E0 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
        ABCD = vsha1cq_u32(ABCD, E1, TMP1);
        TMP1 = vaddq_u32(MSG3, vdupq_n_u32(0x5A827999));
        MSG0 = vsha1su1q_u32(MSG0, MSG3);
        MSG1 = vsha1su0q_u32(MSG1, MSG2, MSG3);

        /* Rounds 8-11 */
        E1 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
        ABCD = vsha1cq_u32(ABCD, E0, TMP0);
        TMP0 = vaddq_u32(MSG0, vdupq_n_u32(0x5A827999));
        MSG1 = vsha1su1q_u32(MSG1, MSG0);
        MSG2 = vsha1su0q_u32(MSG2, MSG3, MSG0);

        /* Rounds 12-15 */
        E0 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
        ABCD = vsha1cq_u32(ABCD, E1, TMP1);
        TMP1 = vaddq_u32(MSG1, vdupq_n_u32(0x6ED9EBA1));
        MSG2 = vsha1su1q_u32(MSG2, MSG1);
        MSG3 = vsha1su0q_u32(MSG3, MSG0, MSG1);

        /* Rounds 16-19 */
        E1 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
        ABCD = vsha1cq_u32(ABCD, E0, TMP0);
        TMP0 = vaddq_u32(MSG2, vdupq_n_u32(0x6ED9EBA1));
        MSG3 = vsha1su1q_u32(MSG3, MSG2);
        MSG0 = vsha1su0q_u32(MSG0, MSG1, MSG2);

        /* Rounds 20-23 */
        E0 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
        ABCD = vsha1pq_u32(ABCD, E1, TMP1);
        TMP1 = vaddq_u32(MSG3, vdupq_n_u32(0x6ED9EBA1));
        MSG0 = vsha1su1q_u32(MSG0, MSG3);
        MSG1 = vsha1su0q_u32(MSG1, MSG2, MSG3);

        /* Rounds 24-27 */
        E1 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
        ABCD = vsha1pq_u32(ABCD, E0, TMP0);
        TMP0 = vaddq_u32(MSG0, vdupq_n_u32(0x6ED9EBA1));
        MSG1 = vsha1su1q_u32(MSG1, MSG0);
        MSG2 = vsha1su0q_u32(MSG2, MSG3, MSG0);

        /* Rounds 28-31 */
        E0 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
        ABCD = vsha1pq_u32(ABCD, E1, TMP1);
        TMP1 = vaddq_u32(MSG1, vdupq_n_u32(0x6ED9EBA1));
        MSG2 = vsha1su1q_u32(MSG2, MSG1);
        MSG3 = vsha1su0q_u32(MSG3, MSG0, MSG1);

        /* Rounds 32-35 */
        E1 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
        ABCD = vsha1pq_u32(ABCD, E0, TMP0);
        TMP0 = vaddq_u32(MSG2, vdupq_n_u32(0x8F1BBCDC));
        MSG3 = vsha1su1q_u32(MSG3, MSG2);
        MSG0 = vsha1su0q_u32(MSG0, MSG1, MSG2);

        /* Rounds 36-39 */
        E0 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
        ABCD = vsha1pq_u32(ABCD, E1, TMP1);
        TMP1 = vaddq_u32(MSG3, vdupq_n_u32(0x8F1BBCDC));
        MSG0 = vsha1su1q_u32(MSG0, MSG3);
        MSG1 = vsha1su0q_u32(MSG1, MSG2, MSG3);

        /* Rounds 40-43 */
        E1 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
        ABCD = vsha1mq_u32(ABCD, E0, TMP0);
        TMP0 = vaddq_u32(MSG0, vdupq_n_u32(0x8F1BBCDC));
        MSG1 = vsha1su1q_u32(MSG1, MSG0);
        MSG2 = vsha1su0q_u32(MSG2, MSG3, MSG0);

        /* Rounds 44-47 */
        E0 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
        ABCD = vsha1mq_u32(ABCD, E1, TMP1);
        TMP1 = vaddq_u32(MSG1, vdupq_n_u32(0x8F1BBCDC));
        MSG2 = vsha1su1q_u32(MSG2, MSG1);
        MSG3 = vsha1su0q_u32(MSG3, MSG0, MSG1);

        /* Rounds 48-51 */
        E1 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
        ABCD = vsha1mq_u32(ABCD, E0, TMP0);
        TMP0 = vaddq_u32(MSG2, vdupq_n_u32(0x8F1BBCDC));
        MSG3 = vsha1su1q_u32(MSG3, MSG2);
        MSG0 = vsha1su0q_u32(MSG0, MSG1, MSG2);

        /* Rounds 52-55 */
        E0 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
        ABCD = vsha1mq_u32(ABCD, E1, TMP1);
        TMP1 = vaddq_u32(MSG3, vdupq_n_u32(0xCA62C1D6));
        MSG0 = vsha1su1q_u32(MSG0, MSG3);
        MSG1 = vsha1su0q_u32(MSG1, MSG2, MSG3);

        /* Rounds 56-59 */
        E1 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
        ABCD = vsha1mq_u32(ABCD, E0, TMP0);
        TMP0 = vaddq_u32(MSG0, vdupq_n_u32(0xCA62C1D6));
        MSG1 = vsha1su1q_u32(MSG1, MSG0);
        MSG2 = vsha1su0q_u32(MSG2, MSG3, MSG0);

        /* Rounds 60-63 */
        E0 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
        ABCD = vsha1pq_u32(ABCD, E1, TMP1);
        TMP1 = vaddq_u32(MSG1, vdupq_n_u32(0xCA62C1D6));
        MSG2 = vsha1su1q_u32(MSG2, MSG1);
        MSG3 = vsha1su0q_u32(MSG3, MSG0, MSG1);

        /* Rounds 64-67 */
        E1 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
        ABCD = vsha1pq_u32(ABCD, E0, TMP0);
        TMP0 = vaddq_u32(MSG2, vdupq_n_u32(0xCA62C1D6));
        MSG3 = vsha1su1q_u32(MSG3, MSG2);
        MSG0 = vsha1su0q_u32(MSG0, MSG1, MSG2);

        /* Rounds 68-71 */
        E0 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
        ABCD = vsha1pq_u32(ABCD, E1, TMP1);
        TMP1 = vaddq_u32(MSG3, vdupq_n_u32(0xCA62C1D6));
        MSG0 = vsha1su1q_u32(MSG0, MSG3);

        /* Rounds 72-75 */
        E1 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
        ABCD = vsha1pq_u32(ABCD, E0, TMP0);

        /* Rounds 76-79 */
        E0 = vsha1h_u32(vgetq_lane_u32(ABCD, 0));
        ABCD = vsha1pq_u32(ABCD, E1, TMP1);

        /* Combine state */
        E0 += E0_SAVED;
        ABCD = vaddq_u32(ABCD_SAVED, ABCD);

        /* Save state */
        vst1q_u32(&state[0], ABCD);
        state[4] = E0;
}


void SHA1Update(SHA1_CTX* context, const unsigned char* data, uint32_t len)
{
    uint32_t i, j;
    j = context->count[0];
    if ((context->count[0] += len << 3) < j)
        context->count[1]++;
    context->count[1] += (len>>29);
    j = (j >> 3) & 63;
    if ((j + len) > 63) {
        memcpy(&context->buffer[j], data, (i = 64-j));
#ifdef ENABLE_ARM
        SHA1Transform_arm64(context->state, context->buffer);
#else
        SHA1Transform(context->state, context->buffer);
#endif
        for ( ; i + 63 < len; i += 64) {
#ifdef ENABLE_ARM
            SHA1Transform_arm64(context->state, &data[i]);
#else
            SHA1Transform(context->state, &data[i]);
#endif
        }
        j = 0;
    }
    else i = 0;
    memcpy(&context->buffer[j], &data[i], len - i);
}

void SHA1Final(unsigned char digest[20], SHA1_CTX* context)
{
    unsigned i;
    unsigned char finalcount[8];
    unsigned char c;
    for (i = 0; i < 8; i++) {
        finalcount[i] = (unsigned char)((context->count[(i >= 4 ? 0 : 1)]
         >> ((3-(i & 3)) * 8) ) & 255);  /* Endian independent */
    }
    c = 0200;
    SHA1Update(context, &c, 1);
    while ((context->count[0] & 504) != 448) {
	c = 0000;
        SHA1Update(context, &c, 1);
    }
    SHA1Update(context, finalcount, 8);  /* Should cause a SHA1Transform() */
    for (i = 0; i < 20; i++) {
        digest[i] = (unsigned char)
         ((context->state[i>>2] >> ((3-(i & 3)) * 8) ) & 255);
    }
    /* Wipe variables */
    memset(context, '\0', sizeof(*context));
    memset(&finalcount, '\0', sizeof(finalcount));
}

void SHA1Init(SHA1_CTX* context)
{
    /* SHA1 initialization constants */
    context->state[0] = 0x67452301;
    context->state[1] = 0xEFCDAB89;
    context->state[2] = 0x98BADCFE;
    context->state[3] = 0x10325476;
    context->state[4] = 0xC3D2E1F0;
    context->count[0] = context->count[1] = 0;
}


#define BUFSIZE 64
int main()
{
    long int start, end;
    SHA1_CTX ctx;
    unsigned char hash[20], buf[BUFSIZE];
    int i;
    for(i=0;i<BUFSIZE;i++)
        buf[i] = i;
    SHA1Init(&ctx);

    start = GetTickCount();
    SHA1Update(&ctx, buf, BUFSIZE);
    SHA1Final(hash, &ctx);
    end = GetTickCount();

    printf("SHA1=");
    for(i=0;i<20;i++)
        printf("%02x", hash[i]);
    printf("\n");

     printf("time cost :%f ms\n", (double)(end - start));
    return 0;
}

