
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include <utility/twi.h>

#include "host.h"

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

static byte reg_read = 0xff;

void
twi_init (void)
{
    host_warn("twi_init called\n");
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

    host_warn("Call to twi_readFrom addr [%x] len [%u] stop [%s]\n",
        addr, len, (stop?"yes":"no"));

    if (!r->dat) {
        host_warn("  Nothing to return!\n");
        exit(1);
    }

    while (reg_read < 0xff && l) {
        switch (reg_read) {
        case 0xfd:
            *data = 0;  /* high byte */
            break;
        case 0xfe:
            *data = r->len;
            break;
        default:
            *data = 0;
        }
        host_warn("  register read from [%02x]: [%02x]\n", 
            reg_read, *data);
        reg_read++; data++; l--;
    }
    if (!l) goto out;

    host_warn("  returning:");
    while (l && r->len) {
        *data = *(r->dat);
        host_warn(" %02x", *data);
        data++; r->dat++;
        l--;    r->len--;
    }
    host_warn("\n  returned [%u] bytes\n", len - l);

    if (!r->len) r++;

  out:
    return len - l;
}

uint8_t
twi_writeTo (uint8_t addr, uint8_t *data, uint8_t len,
    uint8_t wait, uint8_t stop)
{
    host_warn(
        "Call to twi_writeTo addr [%x] len [%u] wait [%s] stop [%s]\n",
        addr, len, (wait?"yes":"no"), (stop?"yes":"no"));

    if (len == 1 && !stop) {
        reg_read = *data;
        host_warn("  set register read [%02x]\n", reg_read);
    }
    else {
        reg_read = 0xff;
        host_warn("  data:");
        while (len) {
            host_warn(" %02x", *data);
            data++;
            len--;
        }
        host_warn("\n");
    }

    return 0;
}
