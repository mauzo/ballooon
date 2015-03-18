#include <stdint.h>

typedef struct pstr {
    unsigned char   len;
    char            dat[1];
} pstr;
typedef const __flash pstr fstr;

#define F(s) ({ \
    static const __flash struct { \
        unsigned char len; \
        char dat[sizeof(s) - 1]; \
    } __Fstr = { sizeof(s), (s) }; \
    (fstr *)&__Fstr; \
})
#define dFSTR(n, s) \
    const __flash struct { \
        unsigned char len; \
        char dat[sizeof(s) - 1]; \
    } n  = { sizeof(s), (s) }
#define dFBYTES(n, ...) \
    const __flash struct { \
        unsigned char len; \
        unsigned char dat[sizeof((unsigned char[]){__VA_ARGS__})]; \
    } n = { sizeof((unsigned char[]){__VA_ARGS__}), {__VA_ARGS__} }

static char buf[81];

static dFSTR(toobig, "Flash string too big");
static dFBYTES(setIOtoUBX, 0x62, 0xB5, 0x01, 0x00);

void panic(fstr *msg);

unsigned char
fstrdup(fstr *s)
{
    unsigned char l, i;

    l = s->len;
    if (l > sizeof buf)
        panic((fstr *)&toobig);
    for (i = 0; i < l; i++)
        buf[i] = s->dat[i];

    return l;
}

extern volatile unsigned char reg;

typedef intptr_t gptr;
#define GPTR(a, f) (((gptr)(a)) | (!!(f)) << 15)
#define GPTR_REF(g, t) ({ \
    const t *__r; \
    const __flash t *__f; \
    ((g & 0x800) \
        ? (__r = (t *)((g) & 0x7fff), *__r) \
        : (__f = (t *)((g) & 0x7fff), *__f)); \
})

void
iovec (gptr adr)
{
    reg = GPTR_REF(adr, unsigned char);
}

void
foo (void)
{
    fstrdup((fstr *)&setIOtoUBX);
    panic(F("foo"));
    iovec(GPTR(F("bar"), 1));
    iovec(GPTR("baz", 0));
}
