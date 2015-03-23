#include "ballooon.h"

#define GPS_ADDR    0x42

static const __flash ubx_cfg_prt set_io_mode = {
    .type           = UBX_TYP_CFG_PRT,
    .len            = ubx_len(ubx_cfg_prt),

    .port           = UBX_CFGPRT_PORT_DDC,
    .mode           = UBX_CFGPRT_DDC_ADDR(GPS_ADDR),
    .in_proto_mask  = UBX_CFGPRT_PROTO_UBX,
    .out_proto_mask = UBX_CFGPRT_PROTO_UBX,
};

int
main (void)
{
    ubx_send(GPS_ADDR, (ubx_pkt *)&set_io_mode);

    return 0;
}
