/* ubx.cpp
 * Low-level communication with the GPS unit.
 */

#include "ballooon.h"

#include <utility/twi.h>

/* Both the AVR and the UBX protocol are little-endian, so there's no
 * need for conversions. We treat the class/ID as a two-byte type field,
 * rather than separating them.
 */

typedef struct ubx_pad {
    uint16_t    sync;
    ubx_pkt     pkt;
} ubx_pad;

static ubx_pad _upad;
static ubx_pad *upad = &_upad;

/* little-endian */
#define UBX_SYNC    0x62b5
#define UBX_EXTRA   (UBX_HEADSIZ + 4)

static void     ubx_cksum   (byte set);

static void
ubx_cksum (byte set)
{
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
ubx_send_packet (ubx_addr adr, ubx_pkt *pkt)
{
    uint16_t len    = dF(pkt).len;

    if (len + UBX_EXTRA > PADSIZ)
        panic(sF("UBX tx packet too long"));

    upad->sync = UBX_SYNC;
    warnf(WDEBUG, "pkt [%04x]", (unsigned int)pkt);

    {
        const __flash char *f = (const char *)aF(pkt);
        warnf(WDEBUG, "f [%04x] start of packet [%02x %02x %02x %02x]",
            (unsigned)f,
            (unsigned)f[0], (unsigned)f[1], (unsigned)f[2], (unsigned)f[3]);
    }

    memcpyF(&upad->pkt, pkt, len + UBX_HEADSIZ);
    ubx_cksum(1);
    len += UBX_EXTRA;

    warnf(WDEBUG, sF("Sending UBX packet type [%x] len [%u]"), 
        dF(pkt).type, len);
    warnf(WDEBUG, "upad [%04x] start of upad [%02x %02x %02x %02x]",
        (unsigned)(char *)upad, (unsigned)((char*)upad)[0], 
        (unsigned)((char*)upad)[1],
        (unsigned)((char*)upad)[2], 
        (unsigned)((char*)upad)[3]);
    pad_dump((char *)upad, len);

    if (twi_writeTo(adr, (byte*)upad, len, 1, 1))
        panic(sF("TWI write failed"));
}

void
ubx_send_with_ack (ubx_addr adr, ubx_pkt *pkt)
{
    ubx_ack ack = { .len = ubx_len(ubx_ack) };

    ubx_send_packet(adr, pkt);
    ubx_recv_packet(adr, (ubx_pkt *)&ack);

    if (ack.type != UBX_TYP_ACK || ack.ack_type != dF(pkt).type)
        panic(sF("UBX didn't get expected ACK"));
    warnf(WDEBUG, sF("Got ack for UBX packet of type [%x]"), ack.ack_type);
}

void
ubx_send_with_reply (ubx_addr adr, ubx_pkt *pkt, uint16_t rlen)
{
    ubx_send_packet(adr, pkt);

    pkt->len = rlen;
    ubx_recv_packet(adr, pkt);
}

void
ubx_setup (void)
{
    warn(WDEBUG, sF("Initialising TWI"));
    twi_init();
}

void
ubx_recv_packet (ubx_addr adr, ubx_pkt *pkt)
{
    byte    want, got;

    want = pkt->len + UBX_EXTRA;
    if (want > PADSIZ)
        panic(sF("UBX rx packet too long"));

    /* It's impossible to do this right with the twi.c interface. What I
     * want to do is read the first part of the packet, check the
     * length, and then read the rest. But twi_readFrom insists on
     * sending a NACK at the end, so I have to do the whole read in one
     * go, so I have to know what length of packet I'm expecting.
     * (readFrom should have a sendNack parameter as well as sendStop.)
     */
    got = twi_readFrom(adr, (byte*)upad, want, 1);

    warnf(WDEBUG, "upad [%x]", (int)upad);
    warnf(WDEBUG, "upad->pkt [%x]", (int)&upad->pkt);
    warnf(WDEBUG, "upad->pkt.type [%x]", (int)upad->pkt.type);

    warnf(WDEBUG, sF("Read a UBX packet of type [%x] len [%u]"), 
        upad->pkt.type, got);
    pad_dump((char *)upad, got);

    if (upad->sync != UBX_SYNC)
        panic(sF("UBX rx lost sync"));
    if (got != want || upad->pkt.len != pkt->len)
        panic(sF("UBX rx packet length incorrect"));
    ubx_cksum(0);

    pkt->type = upad->pkt.type;
    memcpy(pkt->dat, upad->pkt.dat, pkt->len);
}
