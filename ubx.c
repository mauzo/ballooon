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

#define UBX_EXTRA   8

#define UBX_SYNC    0xb562
#define UBX_ACK     0x0501

static void     ubx_cksum   (byte set);

static void
ubx_cksum (byte set)
{
    ubx_pad *upad   = (ubx_pad *)pad;
    char        *p;
    uint16_t    l;
    byte        a = 0, b = 0;

    l = upad->pkt.len;
    for (p = pad + 2; p < pad + l; p++) {
        a += (byte)*p;
        b += a;
    }

    if (set) {
        p[0] = a;
        p[1] = b;
    }
    else {
        if (p[0] != a || p[1] != b)
            panic(F("UBX packet checksum failed"));
    }
}

void
ubx_send_packet (ubx_addr adr, ubx_pkt *pkt)
{
    ubx_pad *upad   = (ubx_pad *)pad;

    if (pkt->len + UBX_EXTRA > PADSIZ)
        panic(F("UBX tx packet too long"));

    upad->sync      = UBX_SYNC;
    upad->pkt.type  = pkt->type;
    upad->pkt.len   = pkt->len;
    memcpy(upad->pkt.dat, pkt->dat, pkt->len);
    ubx_cksum(1);

    if (twi_writeTo(adr, (byte*)pad, pkt->len + 8, 1, 1))
        panic(F("TWI write failed"));
}

void
ubx_send_with_ack (ubx_addr adr, ubx_pkt *pkt)
{
    ubx_send_packet(adr, pkt);
    ubx_recv_ack(adr, pkt);
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
ubx_recv_ack (ubx_addr adr, ubx_pkt *pkt)
{
    ubx_ack ack;

    ack.len = 2;
    ubx_recv_packet(adr, (ubx_pkt *)&ack);

    if (ack.type != UBX_ACK || ack.ack_type != pkt->type)
        panic(F("UBX didn't get expected ACK"));
}

/* Receive a packet into ubx_pkt; the payload is stored in the pad. 
 * pkt->len must be set on input to the expected length of the packet.
 */
void
ubx_recv_packet (ubx_addr adr, ubx_pkt *pkt)
{
    ubx_pad *upad = (ubx_pad *)pad;
    byte    want, got;

    want = pkt->len + UBX_EXTRA;
    if (want > PADSIZ)
        panic(F("UBX rx packet too long"));

    /* It's impossible to do this right with the twi.c interface. What I
     * want to do is read the first part of the packet, check the
     * length, and then read the rest. But twi_readFrom insists on
     * sending a NACK at the end, so I have to do the whole read in one
     * go, so I have to know what length of packet I'm expecting.
     * (readFrom should have a sendNack parameter as well as sendStop.)
     */
    got = twi_readFrom(adr, (byte*)pad, want, 1);
    if (got != want || upad->pkt.len != pkt->len)
        panic(F("UBX rx packet length incorrect"));
    
    if (upad->sync != UBX_SYNC)
        panic(F("UBX rx lost sync"));

    ubx_cksum(0);

    pkt->type = upad->pkt.type;
    memcpy(pkt->dat, upad->pkt.dat, pkt->len);
}
