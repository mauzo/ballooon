/* ubx.h
 * Low-level UBX communication.
 */

#ifndef __UBX_H
#define __UBX_H

typedef byte ubx_addr;

    /* pkt->len must be set to the maximum expected length.
     * If pkt->type is set, it will be checked against the packet. */
void    ubx_recv            (ubx_addr adr, ubx_pkt *pkt);
void    ubx_send            (ubx_addr adr, ubx_pkt *pkt);
void    ubx_send_with_ack   (ubx_addr adr, ubx_pkt *pkt);
void    ubx_setup           (void);

#endif
