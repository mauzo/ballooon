#include "ballooon.h"

typedef struct ubx_pad {
    uint16_t    sync;
    ubx_pkt     pkt;
} ubx_pad;

static byte     _upad[PADSIZ];

/* little-endian */
#define UBX_SYNC        0x62b5
#define UBX_SYNCSIZ     (UBX_HEADSIZ + 2)
#define UBX_CKSIZ       2
#define UBX_EXTRA       (UBX_SYNCSIZ + UBX_CKSIZ)

#define UBX_MAXDROP     10

#define TWI_BUFSIZ  32

#define SHOW_PTR(p, n) ({ \
    warnf(WDEBUG, sF(n " [%lx] = [%02x %02x %02x %02x %02x %02x]"), \
        (unsigned long)(p), \
        (unsigned)dF((byte*)p), (unsigned)dF(((byte*)p)+1), \
        (unsigned)dF(((byte*)p)+2), (unsigned)dF(((byte*)p)+3), \
        (unsigned)dF(((byte*)p)+4), (unsigned)dF(((byte*)p)+5)); \
})


static int
twi_writeTo(byte addr, byte *data, byte len, byte wait, byte stop)
{
    return 1;
}

static void
ubx_cksum (byte set)
{
    ubx_pad *upad = (ubx_pad *)_upad;
    byte    *p, *e, a = 0, b = 0;

    e = upad->pkt.dat + upad->pkt.len;
    for (p = (byte *)&upad->pkt; p < e; p++) {
        a += *p;
        b += a;
    }

    if (set) {
        p[0] = a;
        p[1] = b;
    }
    else {
        if (p[0] != a || p[1] != b)
            panic(sF("UBX packet checksum failed"));
    }
}

void
ubx_send (ubx_addr adr, ubx_pkt *pkt)
{
    ubx_pad     *upad   = (ubx_pad *)_upad;
    uint16_t    len     = dF(pkt).len;
    byte        *p, err;

    if (len + UBX_EXTRA > PADSIZ)
        panic(sF("UBX tx packet too long"));

    upad->sync = UBX_SYNC;
    SHOW_PTR(pkt, "ubx_send: pkt");

    memcpyF(&upad->pkt, pkt, len + UBX_HEADSIZ);
    ubx_cksum(1);
    len += UBX_EXTRA;

    warnf(WDEBUG, sF("Sending UBX packet type [%02x] len [%u]"), 
        dF(pkt).type, len);
    SHOW_PTR(upad, "ubx_send: upad");
    pad_dump(sF("ubx_set: upad"), (char *)upad, len);

    /* twi is really annoying: rather than using the buffer we give it,
     * it copies the data into a 32-byte buffer of its own. So don't try
     * to send more than 32b at once. */
    p = (byte *)upad;
    while (len > TWI_BUFSIZ) {
        if (err = twi_writeTo(adr, p, TWI_BUFSIZ, 1, 0))
            panic(sF("TWI partial write failed [%u]"), (unsigned)err);
        p   += TWI_BUFSIZ;
        len -= TWI_BUFSIZ;
    }
    if (err = twi_writeTo(adr, p, len, 1, 1))
        panic(sF("TWI final write failed [%u]"), (unsigned)err);
}
