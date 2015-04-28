/* gps.cpp
 * Functions for talking to the GPS module.
 */

#include <Arduino.h>

#include "ubx.h"
#include "gps.h"
#include "task.h"
#include "warn.h"

gps_data    gps_last_fix;

/* XXX This doesn't work in C++ (grr)
static const ubx_cfg_prt set_io_mode = {
    .type           = UBX_TYP_CFG_PRT,
    .len            = ubx_len(ubx_cfg_prt),

    .port           = UBX_CFGPRT_PORT_DDC,
    .mode           = UBX_CFGPRT_DDC_ADDR(GPS_ADDR),
    .in_proto_mask  = UBX_CFGPRT_PROTO_UBX,
    .out_proto_mask = UBX_CFGPRT_PROTO_UBX,
};
*/

static byte     set_io_mode[] = {
    0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x01, 0x00, 0x01, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0xA0, 0x96
};
static byte     set_nav_mode[] = {
    0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF,
    0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 
    0x00, 0x00, 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 
    0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
    0x00, 0x00, 0x16, 0xDC
};
static byte     req_nav_pvt[] = {
    0xB5, 0x62, 0x01, 0x07, 0x00, 0x00, 0x08, 0x19
};

static void     gps_log_fix     (gps_data *g);
static void     gps_parse_pvt   (ubx_nav_pvt *pkt, gps_data *g);
static byte     gps_verify_lock (ubx_nav_pvt *pkt);
static wchan    gps_setup       (void);
static wchan    gps_run         (wchan now);

task gps_task = {
    .name   = "GPS",
    .when   = TASK_STOP,

    .setup  = gps_setup,
    .run    = gps_run,
    .reset  = 0,
};

static wchan
gps_run (wchan now)
{
    ubx_nav_pvt *pkt;

    warn(WDEBUG, "Requesting NAV-PVT.");
    if(!sendUBX(GPS_ADDR, req_nav_pvt, sizeof req_nav_pvt)) {
        warn(WERROR, "Error sending NAV-PVT");
        goto fail;
    }

    if (!(pkt = (ubx_nav_pvt *)ubx_read(GPS_ADDR))) {
        warn(WWARN, "Failed to read UBX packet");
        goto fail;
    }

    if (!gps_verify_lock(pkt)) {
        warn(WWARN, "Packet failed to verify");
        goto fail;
    }
        
    gps_parse_pvt(pkt, &gps_last_fix);
    gps_log_fix(&gps_last_fix);
    swi(SWI_GPS);

    return TASK_TIME(now, 10000);

  fail:
    warn(WWARN, "Failed to get NAV-PVT");
    return TASK_TIME(now, 3000);
}

static wchan
gps_setup (void)
{
    warn(WNOTICE, "Setting IO protocols.");
    if(!sendUBX(GPS_ADDR, set_io_mode, sizeof set_io_mode)) {
        warn(WERROR, "Error sending CFG-PRT.");
        return TASK_STOP;
    }
    
    warn(WDEBUG, "Awaiting confirmation.");
    if(!getUBX_ACK(GPS_ADDR, set_io_mode)) {
        warn(WERROR, "Error: no confirmation received for CFG-PRT.");
        return TASK_STOP;
    }
    
    warn(WNOTICE, "Setting flight mode (1g).");
    if(!sendUBX(GPS_ADDR, set_nav_mode, sizeof set_nav_mode)) {
        warn(WERROR, "Error sending CFG-NAV.");
        return TASK_STOP;
    }
    
    warn(WNOTICE, "Awaiting confirmation.");
    if(!getUBX_ACK(GPS_ADDR, set_nav_mode)) {
        warn(WERROR, "Error: no confirmation received for CFG-NAV.");
        return TASK_STOP;
    }

    return TASK_RUN;
}

void 
gps_log_fix (gps_data *g) 
{
    warn(WLOG, "GPS Data:");
    warnf(WLOG, "Time: %02u:%02u:%02u", g->hr, g->min, g->sec);
    warnf(WLOG, "Lat: %li, Lon: %li, Alt: %li", g->lat, g->lon, g->alt);
    warnf(WLOG, "Number of satellites used: %u", g->num_sat);
    warnf(WLOG, "Type of lock: %u", g->fix_type);
}

// Good fix is > 4 sats and fixType = 3 or 4.
byte
gps_verify_lock (ubx_nav_pvt *pkt)
{
    warnf(WDEBUG, "Received packet type [%04x]", pkt->type);
    if (pkt->type != UBX_TYP_NAV_PVT) {
        warnf(WWARN, "Not a NAV-PVT packet");
        goto fail;
    }

    if (pkt->iTOW == gps_last_fix.itow) {
        warn(WWARN, "Same fix as last time");
        goto fail;
    }
    if (pkt->fix_type != 3 && pkt->fix_type != 4) {
        warnf(WWARN, "Invalid fix type [%u]", pkt->fix_type);
        goto fail;
    }
    if (pkt->num_sv < 4) {
        warnf(WWARN, "Not enough satellites [%u]", pkt->num_sv);
        goto fail;
    }

    if (!gps_last_fix.itow)
        warn(WNOTICE, "Got GPS lock");

    return 1;

  fail:
    if (gps_last_fix.itow) {
        warn(WNOTICE, "GPS lock lost");
        gps_last_fix.itow = 0;
    }

    return 0;
}

// Chops out only the data I want from the UBX sentence
static void
gps_parse_pvt (ubx_nav_pvt *pkt, gps_data *g)
{
    g->itow     = pkt->iTOW;
    g->when     = millis();

    g->fix_type = pkt->fix_type;
    g->num_sat  = pkt->num_sv;

    g->lon      = pkt->lon / 100;
    g->lat      = pkt->lat / 100;
    g->alt      = pkt->height / 100;

    if (pkt->valid & UBX_NAVPVT_VALID_TIME) {
        g->hr   = pkt->hour;
        g->min  = pkt->min;
        g->sec  = pkt->sec;
    }
}
