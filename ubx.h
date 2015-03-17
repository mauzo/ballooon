/* ubx.h
 * Low-level UBX communication.
 */

#ifndef __UBX_H
#define __UBX_H

typedef byte ubx_addr;

void    ubx_send_packet     (ubx_addr adr, ubx_pkt *pkt);
void    ubx_send_with_ack   (ubx_addr adr, ubx_pkt *pkt);
    /* rlen is the expected payload length of the reply */
void    ubx_send_with_reply (ubx_addr adr, ubx_pkt *pkt, uint16_t rlen);
void    ubx_recv_ack        (ubx_addr adr, ubx_pkt *pkt);
    /* pkt->len must be set to the expected length */
void    ubx_recv_packet     (ubx_addr adr, ubx_pkt *pkt);

#endif
