#include "tiffiop.h"

void tp_TIFFSwabShort(uint16* wp)
{
    register unsigned char* cp = (unsigned char*) wp;
    unsigned char t;
    assert(sizeof(uint16)==2);
    t = cp[1]; cp[1] = cp[0]; cp[0] = t;
}

void tp_TIFFSwabLong(uint32* lp)
{
    register unsigned char* cp = (unsigned char*) lp;
    unsigned char t;
    assert(sizeof(uint32)==4);
    t = cp[3]; cp[3] = cp[0]; cp[0] = t;
    t = cp[2]; cp[2] = cp[1]; cp[1] = t;
}

void tp_TIFFSwabLong8(uint64* lp)
{
    register unsigned char* cp = (unsigned char*) lp;
    unsigned char t;
    assert(sizeof(uint64)==8);
    t = cp[7]; cp[7] = cp[0]; cp[0] = t;
    t = cp[6]; cp[6] = cp[1]; cp[1] = t;
    t = cp[5]; cp[5] = cp[2]; cp[2] = t;
    t = cp[4]; cp[4] = cp[3]; cp[3] = t;
}

void tp_TIFFSwabArrayOfShort(register uint16* wp, tmsize_t n)
{
    register unsigned char* cp;
    register unsigned char t;
    assert(sizeof(uint16)==2);
    /* XXX unroll loop some */
    while (n-- > 0) {
        cp = (unsigned char*) wp;
        t = cp[1]; cp[1] = cp[0]; cp[0] = t;
        wp++;
    }
}

void tp_TIFFSwabArrayOfTriples(register uint8* tp, tmsize_t n)
{
    unsigned char* cp;
    unsigned char t;

    /* XXX unroll loop some */
    while (n-- > 0) {
        cp = (unsigned char*) tp;
        t = cp[2]; cp[2] = cp[0]; cp[0] = t;
        tp += 3;
    }
}

void tp_TIFFSwabArrayOfLong(register uint32* lp, tmsize_t n)
{
    register unsigned char *cp;
    register unsigned char t;
    assert(sizeof(uint32)==4);
    /* XXX unroll loop some */
    while (n-- > 0) {
        cp = (unsigned char *)lp;
        t = cp[3]; cp[3] = cp[0]; cp[0] = t;
        t = cp[2]; cp[2] = cp[1]; cp[1] = t;
        lp++;
    }
}

void tp_TIFFSwabArrayOfLong8(register uint64* lp, tmsize_t n)
{
    register unsigned char *cp;
    register unsigned char t;
    assert(sizeof(uint64)==8);
    /* XXX unroll loop some */
    while (n-- > 0) {
        cp = (unsigned char *)lp;
        t = cp[7]; cp[7] = cp[0]; cp[0] = t;
        t = cp[6]; cp[6] = cp[1]; cp[1] = t;
        t = cp[5]; cp[5] = cp[2]; cp[2] = t;
        t = cp[4]; cp[4] = cp[3]; cp[3] = t;
        lp++;
    }
}

void tp_TIFFSwabFloat(float* fp)
{
    register unsigned char* cp = (unsigned char*) fp;
    unsigned char t;
    assert(sizeof(float)==4);
    t = cp[3]; cp[3] = cp[0]; cp[0] = t;
    t = cp[2]; cp[2] = cp[1]; cp[1] = t;
}

void tp_TIFFSwabArrayOfFloat(register float* fp, tmsize_t n)
{
    register unsigned char *cp;
    register unsigned char t;
    assert(sizeof(float)==4);
    /* XXX unroll loop some */
    while (n-- > 0) {
        cp = (unsigned char *)fp;
        t = cp[3]; cp[3] = cp[0]; cp[0] = t;
        t = cp[2]; cp[2] = cp[1]; cp[1] = t;
        fp++;
    }
}

void tp_TIFFSwabDouble(double *dp)
{
    register unsigned char* cp = (unsigned char*) dp;
    unsigned char t;
    assert(sizeof(double)==8);
    t = cp[7]; cp[7] = cp[0]; cp[0] = t;
    t = cp[6]; cp[6] = cp[1]; cp[1] = t;
    t = cp[5]; cp[5] = cp[2]; cp[2] = t;
    t = cp[4]; cp[4] = cp[3]; cp[3] = t;
}

void tp_TIFFSwabArrayOfDouble(double* dp, tmsize_t n)
{
    register unsigned char *cp;
    register unsigned char t;
    assert(sizeof(double)==8);
    /* XXX unroll loop some */
    while (n-- > 0) {
        cp = (unsigned char *)dp;
        t = cp[7]; cp[7] = cp[0]; cp[0] = t;
        t = cp[6]; cp[6] = cp[1]; cp[1] = t;
        t = cp[5]; cp[5] = cp[2]; cp[2] = t;
        t = cp[4]; cp[4] = cp[3]; cp[3] = t;
        dp++;
    }
}
