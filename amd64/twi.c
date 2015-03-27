
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
    { 0, NULL },
};

void
twi_init (void)
{ }

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

    printf("Call to twi_readFrom addr [%x] len [%u] stop [%s]\n",
        addr, len, (stop?"yes":"no"));

    if (!r->dat) {
        printf("  Nothing to return!\n");
        exit(1);
    }

    while (l && r->len) {
        *data = *(r->dat);
        printf(" %02x", *data);
        data++; r->dat++;
        l--;    r->len--;
    }
    printf("\n  returned [%u] bytes\n", len - l);

    if (!r->len) r++;

    return len - l;
}

uint8_t
twi_writeTo (uint8_t addr, uint8_t *data, uint8_t len,
    uint8_t wait, uint8_t stop)
{
    printf("Call to twi_writeTo addr [%x] len [%u] wait [%s] stop [%s]\n"
        "  data:",
        addr, len, (stop?"yes":"no"), (wait?"yes":"no"));
    while (len) {
        printf(" %02x", *data);
        data++;
        len--;
    }
    printf("\n");

    return 0;
}
