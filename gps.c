/* gps.c
 * Talk to the GPS unit over UBX.
 */

#include "ballooon.h"

#define GPS_ADDR    0x42

static void     gps_setup       (void);
static void     gps_run         (long now);
static void     gps_reset       (void);

static byte     gps_fetch_data  (void);
static void     gps_print_data  (void);
static byte     gps_validate    (ubx_nav_pvt *nav);

gps_fix gps_last_fix    = { 
    .when = 0,
};

task    gps_task        = {
    .name       = "GPS",
    .when       = TASK_INACTIVE,

    .setup      = gps_setup,
    .run        = gps_run,
    .reset      = gps_reset
};

static const __flash ubx_cfg_prt set_io_mode = {
    .type           = UBX_TYP_CFG_PRT,
    .len            = ubx_len(ubx_cfg_prt),

    .port           = UBX_CFGPRT_PORT_DDC,
    .mode           = UBX_CFGPRT_DDC_ADDR(GPS_ADDR),
    .in_proto_mask  = UBX_CFGPRT_PROTO_UBX,
    .out_proto_mask = UBX_CFGPRT_PROTO_UBX,
};

static const __flash ubx_cfg_nav5 set_nav_mode = {
    .type           = UBX_TYP_CFG_NAV5,
    .len            = ubx_len(ubx_cfg_nav5),

    .mask           = UBX_CFGNAV5_MASK_ALL,
    .dyn_model      = UBX_CFGNAV5_DYN_AIRBORNE1G,
    .fix_mode       = UBX_CFGNAV5_FIX_AUTO,
    .fixed_alt_var  = 10000,
    .min_elev       = 5,
    .p_dop          = 250,
    .t_dop          = 250,
    .p_acc          = 100,
    .t_acc          = 300,
};

static const __flash ubx_cfg_rst reset_gps = {
    .type           = UBX_TYP_CFG_RST,
    .len            = ubx_len(ubx_cfg_rst),

    .nav_bbr_mask   = UBX_CFGRST_BBR_HOT,
    .reset_mode     = UBX_CFGRST_RST_SOFT,
};

static void
gps_setup (void)
{
    ubx_setup();

    ubx_send_with_ack(GPS_ADDR, (ubx_pkt *)pF(set_io_mode));
    ubx_send_with_ack(GPS_ADDR, (ubx_pkt *)pF(set_nav_mode));

    gps_task.when = TASK_START;
}

static void
gps_run (long now)
{
    if (gps_fetch_data())
        gps_print_data();
    else
        warn(sF("GPS fetch failed"));

    gps_task.when = now + 10000;
}

static void
gps_reset (void)
{
    /* XXX I don't know if this gets an ACK or not. There's nothing in
     * the documentation to suggest it doesn't. */
    ubx_send_with_ack(GPS_ADDR, (ubx_pkt *)pF(reset_gps));

    /* This will also reinitialise the TWI stuff, which is probably a
     * good idea, unless it breaks something... */
    gps_setup();
}

static byte
gps_fetch_data (void)
{
    /* The request packet has length 0, but we allocate the full
     * structure for the reply. */
    ubx_nav_pvt nav = {
        .type   = UBX_TYP_NAV_PVT,
        .len    = 0,
    };

    /* The response should be full-length. */
    ubx_send_with_reply(GPS_ADDR, (ubx_pkt *)&nav, ubx_len(nav));

    if (nav.type != UBX_TYP_NAV_PVT)
        panic(sF("GPS got wrong response to NAV-PVT"));

    if (!gps_validate(&nav))
        return 0;

    if (nav.valid & UBX_NAVPVT_VALID_TIME) {
        gps_last_fix.hr     = nav.hour;
        gps_last_fix.min    = nav.min;
        gps_last_fix.sec    = nav.sec;
    }
    gps_last_fix.lat    = nav.lat / 100;
    gps_last_fix.lon    = nav.lon / 100;
    gps_last_fix.alt    = nav.height / 100;

    gps_last_fix.when       = millis();
    gps_last_fix.nsats      = nav.num_sv;
    gps_last_fix.fix_type   = nav.fix_type;

    return 1;
}

void
gps_print_data (void)
{
    warn(sF("\r\nGPS Data:"));
    warnf(sF("Time: %02u:%02u:%02u"),
        gps_last_fix.hr, gps_last_fix.min, gps_last_fix.sec);
    warnf(sF("Lat: %li, Lon: %li"), gps_last_fix.lat, gps_last_fix.lon);
    warnf(sF("Altitude: %li"), gps_last_fix.alt);
    warnf(sF("Number of satellites used: %u"), gps_last_fix.nsats);
    warnf(sF("Type of lock: %u\n"), gps_last_fix.fix_type);
}

static byte
gps_validate (ubx_nav_pvt *nav)
{
    if (nav->fix_type != UBX_NAVPVT_FIX_3D
        && nav->fix_type != UBX_NAVPVT_FIX_BOTH)
        return 0;

    if (nav->num_sv < 4)
        return 0;

    return 1;
}
