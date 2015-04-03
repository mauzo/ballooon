/* ubx.h
 * Functions for talking to UBX devices.
 */

#ifndef __UBX_H
#define __UBX_H

#include "ubx_pkt.h"

// set to maximum message you want to receive
#define UBX_BUFSIZ          100
#define UBX_SYNC_CHAR1      0xB5
#define UBX_SYNC_CHAR2      0x62

/* This is just for clarity: a UBX address is just a byte, but it's
 * easier to understand function prototypes if we give it its own type.
 */
typedef int    ubx_addr;

boolean     getUBX_ACK      (ubx_addr adr, byte *msg);
boolean     sendUBX         (ubx_addr adr, byte *msg, byte len);
void        ubx_setup       (void);
ubx_pkt *   ubx_read        (ubx_addr adr);

#endif
