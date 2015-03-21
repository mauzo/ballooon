/* ubx.cpp
 * Low-level communication with the GPS unit.
 */

#include "ballooon.h"

#include <utility/twi.h>

/* Why can't people use standard names for things? */
#define TWI_BUFSIZ TWI_BUFFER_LENGTH

/* Both the AVR and the UBX protocol are little-endian, so there's no
 * need for conversions. We treat the class/ID as a two-byte type field,
 * rather than separating them.
 */

typedef struct ubx_pad {
    uint16_t    sync;
    ubx_pkt     pkt;
} ubx_pad;

static byte     _upad[PADSIZ];
static byte     dropped;

/* little-endian */
#define UBX_SYNC        0x62b5
#define UBX_SYNCSIZ     (UBX_HEADSIZ + 2)
#define UBX_CKSIZ       2
#define UBX_EXTRA       (UBX_SYNCSIZ + UBX_CKSIZ)

#define UBX_MAXDROP     10

#define SHOW_PTR(p, n) ({ \
    warnf(WDEBUG, sF(n " [%04x] = [%02x %02x %02x %02x %02x %02x]"), \
        (unsigned)(p), \
        (unsigned)dF((byte*)p), (unsigned)dF(((byte*)p)+1), \
        (unsigned)dF(((byte*)p)+2), (unsigned)dF(((byte*)p)+3), \
        (unsigned)dF(((byte*)p)+4), (unsigned)dF(((byte*)p)+5)); \
})

static void     ubx_cksum   (byte set);
static void     ubx_drop    (ubx_addr adr, byte len);
static void     ubx_twi_read (ubx_addr adr, byte **p, byte len, byte stop);

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

static void
ubx_drop (ubx_addr adr, byte len)
{
    byte    buf[TWI_BUFSIZ], *p;

    while (len > TWI_BUFSIZ) {
        p = buf;
        ubx_twi_read(adr, &p, TWI_BUFSIZ, 0);
        len -= TWI_BUFSIZ;
    }
    p = buf;
    ubx_twi_read(adr, &p, len, 1);

    if (++dropped > UBX_MAXDROP)
        panic(sF("UBX dropped too many packets"));
}

static void
ubx_twi_read (ubx_addr adr, byte **p, byte len, byte stop)
{
    byte    got;

    while (len > 0) {
        if (len > TWI_BUFSIZ)
            got = twi_readFrom(adr, *p, TWI_BUFSIZ, 0);
        else
            got = twi_readFrom(adr, *p, len, stop);

        /* twi doesn't return an error on read, so if we get no data
         * there's no way to recover */
        if (!got)
            panic(sF("TWI read returned no data"));

        *p  += got;
        len -= got;
    }
}

void
ubx_recv (ubx_addr adr, ubx_pkt *pkt)
{
    ubx_pad *upad   = (ubx_pad *)_upad;
    byte    *p, len;

    p   = (byte *)upad;
    ubx_twi_read(adr, &p, UBX_SYNCSIZ, 0);
    if (upad->sync != UBX_SYNC)
        panic(sF("UBX rx lost sync"));

    len = upad->pkt.len;
    warnf(WDEBUG, sF("Reading a UBX packet of type [%02x] len [%u]"), 
        upad->pkt.type, len);
    SHOW_PTR(upad, "ubx_recv_packet: header");
    pad_dump((char *)upad, len + UBX_SYNCSIZ);

    if (len > pkt->len || len + UBX_EXTRA > PADSIZ) {
        warn(WWARN, sF("UBX packet too long, dropping"));
        goto drop;
    }
    if (pkt->type && upad->pkt.type != pkt->type) {
        warnf(WWARN, sF("Unexpected UBX packet of type [%02x], dropping"),
            upad->pkt.type);
        goto drop;
    }

    ubx_twi_read(adr, &p, len + UBX_CKSIZ, 1);
    pad_dump((char *)upad, len + UBX_EXTRA);
    ubx_cksum(0);

    memcpy(pkt, &upad->pkt, len + UBX_HEADSIZ);
    SHOW_PTR(pkt, "ubx_recv_packet: pkt");
    pad_dump((char *)pkt, len + UBX_HEADSIZ);
    return;

drop:
    ubx_drop(adr, len + UBX_CKSIZ);
    return;
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
    SHOW_PTR(pkt, "ubx_send_packet: pkt");

    memcpyF(&upad->pkt, pkt, len + UBX_HEADSIZ);
    ubx_cksum(1);
    len += UBX_EXTRA;

    warnf(WDEBUG, sF("Sending UBX packet type [%02x] len [%u]"), 
        dF(pkt).type, len);
    SHOW_PTR(upad, "ubx_send_packet: upad");
    pad_dump((char *)upad, len);

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

void
ubx_send_with_ack (ubx_addr adr, ubx_pkt *pkt)
{
    ubx_ack ack = {
        .type   = UBX_TYP_ACK,
        .len    = ubx_len(ubx_ack), 
    };

    ubx_send(adr, pkt);
    ubx_recv(adr, (ubx_pkt *)&ack);

    warnf(WDEBUG, sF("Got ack for UBX packet of type [%02x]"), ack.ack_type);
    if (ack.ack_type != dF(pkt).type)
        panic(sF("UBX didn't get expected ACK"));
}

void
ubx_setup (void)
{
    warn(WDEBUG, sF("Initialising TWI"));
    twi_init();
    dropped = 0;
}

