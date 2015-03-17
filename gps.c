/* gps.c
 * Talk to the GPS unit over UBX.
 */

#include "ballooon.h"

#define GPS_ADDR    0x42

static void     gps_setup       (void);
static void     gps_run         (long now);
static void     gps_reset       (void);

task gps_task = {
    .name       = "GPS",
    .when       = TASK_INACTIVE,

    .setup      = gps_setup,
    .run        = gps_run,
    .reset      = gps_reset
};

static ubx_cfg_prt set_io_mode = {
    .type           = UBX_TYP_CFG_PRT,
    .len            = sizeof(ubx_cfg_prt),
    .mode           = UBX_CFGPRT_MODE_8BIT | UBX_CFGPRT_MODE_NPAR,
    .in_proto_mask  = UBX_CFGPRT_PROTO_UBX,
    .out_proto_mask = UBX_CFGPRT_PROTO_UBX,
};

static void
gps_setup (void)
{
    ubx_send_with_ack(GPS_ADDR, (ubx_pkt *)&set_io_mode);

    gps_task.when = TASK_START;
}

static void
gps_run (long now)
{
    gps_task.when = now + 10000;
}

static void
gps_reset (void)
{
    gps_task.when = TASK_START;
}
