
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <utility/twi.h>

typedef unsigned char byte;

struct reply {
    byte    len;
    byte    *dat;
};
#define R(x) { sizeof(x) - 1, (byte *)(x) }

static struct reply replies[] = {
    R("\xb5\x62\x05\x01\x02\x00\x06\x00\x0e\x37"),
    R("\xb5\x62\x05\x01\x02\x00\x06\x24\x32\x5b"),
    R("\xb5\x62\x01\x07\x54\x00\x00\x00\x00\x00\xdf\x07\x03\x1c\x09\x05\x15\x07\xc8\x00\x00\x00\x40\xe2\x01\x00\x03\x01\x00\x05\x40\x39\xd2\xff\x80\x58\x4f\x00\xe8\x03\x00\x00\xe8\x03\x00\x00\x64\x00\x00\x00\x64\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x64\x00\x00\x00\x64\x00\x00\x00\xe8\x03\x00\x00\x00\x00\x00\x00\x41\x17"),
    { 0, NULL },
};

void
twi_init (void)
{
    fprintf(stderr, "twi_init called\n");
}

uint8_t
twi_transmit (const uint8_t *data, uint8_t length)
{
    return 0;
}

uint8_t
twi_readFrom (uint8_t addr, uint8_t *data, uint8_t len, uint8_t stop)
{
    static struct reply *r = replies;
    byte l = len;

    fprintf(stderr, "Call to twi_readFrom addr [%x] len [%u] stop [%s]\n",
        addr, len, (stop?"yes":"no"));

    if (!r->dat) {
        fprintf(stderr, "  Nothing to return!\n");
        exit(1);
    }

    fprintf(stderr, "  returning:");
    while (l && r->len) {
        *data = *(r->dat);
        fprintf(stderr, " %02x", *data);
        data++; r->dat++;
        l--;    r->len--;
    }
    fprintf(stderr, "\n  returned [%u] bytes\n", len - l);

    if (!r->len) r++;

    return len - l;
}

uint8_t
twi_writeTo (uint8_t addr, uint8_t *data, uint8_t len,
    uint8_t wait, uint8_t stop)
{
    fprintf(stderr,
        "Call to twi_writeTo addr [%x] len [%u] wait [%s] stop [%s]\n"
        "  data:",
        addr, len, (stop?"yes":"no"), (wait?"yes":"no"));
    while (len) {
        fprintf(stderr, " %02x", *data);
        data++;
        len--;
    }
    fprintf(stderr, "\n");

    return 0;
}
