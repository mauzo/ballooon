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

/* little-endian */
#define UBX_SYNC    0x62b5
#define UBX_EXTRA   (UBX_HEADSIZ + 4)

static void     ubx_cksum   (byte set);

static void
ubx_cksum (byte set)
{
    ubx_pad *upad = (ubx_pad *)pad;
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
    ubx_pad *upad   = (ubx_pad *)pad;
    uint16_t len    = dF(pkt).len;

    if (len + UBX_EXTRA > PADSIZ)
        panic(sF("UBX tx packet too long"));

    upad->sync = UBX_SYNC;
    memcpyF(&upad->pkt, pkt, len + UBX_HEADSIZ);
    ubx_cksum(1);

    if (twi_writeTo(adr, (byte*)pad, len + UBX_EXTRA, 1, 1))
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
    twi_init();
}

void
ubx_recv_packet (ubx_addr adr, ubx_pkt *pkt)
{
    ubx_pad *upad = (ubx_pad *)pad;
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
    got = twi_readFrom(adr, (byte*)pad, want, 1);
    if (got != want || upad->pkt.len != pkt->len)
        panic(sF("UBX rx packet length incorrect"));
    
    if (upad->sync != UBX_SYNC)
        panic(sF("UBX rx lost sync"));

    ubx_cksum(0);

    pkt->type = upad->pkt.type;
    memcpy(pkt->dat, upad->pkt.dat, pkt->len);
}
