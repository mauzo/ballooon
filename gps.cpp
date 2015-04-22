/* gps.cpp
 * Functions for talking to the GPS module.
 */

#include <Arduino.h>

#include "ubx.h"
#include "gps.h"
#include "task.h"
#include "warn.h"

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

byte setIOtoUBX[] = {0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00,
    0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0xA0, 0x96}; 
byte airborne1g[] = {0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF,
    0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x05, 0x00,
    0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x16, 0xDC};
// 0x10 0x27 0x00 0x00 for 'fixed altitude variance? Time and pos masks?
byte reqNAV_PVT[] =  {0xB5, 0x62, 0x01, 0x07, 0x00, 0x00, 0x08, 0x19};

static GPS_DATA         gpsData;
static GPS_DATA         lastKnownFix;
static boolean          gpsLock         = false;
static unsigned long    gpsCheckTime;

static byte     gps_parse_pvt   (ubx_nav_pvt *pkt);
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
    if(!sendUBX(GPS_ADDR, reqNAV_PVT, sizeof(reqNAV_PVT)))
        warn(WERROR, "Error sending NAV-PVT");

    pkt = (ubx_nav_pvt *)ubx_read(GPS_ADDR);
    if (pkt && gps_parse_pvt(pkt)) {
        checkForLock();
        printGPSData();
        return TASK_TIME(now, 10000);
    }

    return TASK_TIME(now, 3000);
}

static wchan
gps_setup (void)
{
    warn(WNOTICE, "Setting IO protocols.");
    if(!sendUBX(GPS_ADDR, setIOtoUBX, sizeof(setIOtoUBX))) {
        warn(WERROR, "Error sending CFG-PRT.");
        return TASK_STOP;
    }
    
    warn(WDEBUG, "Awaiting confirmation.");
    if(!getUBX_ACK(GPS_ADDR, setIOtoUBX)) {
        warn(WERROR, "Error: no confirmation received for CFG-PRT.");
        return TASK_STOP;
    }
    
    warn(WNOTICE, "Setting flight mode (1g).");
    if(!sendUBX(GPS_ADDR, airborne1g, sizeof(airborne1g))) {
        warn(WERROR, "Error sending CFG-NAV.");
        return TASK_STOP;
    }
    
    warn(WNOTICE, "Awaiting confirmation.");
    if(!getUBX_ACK(GPS_ADDR, airborne1g)) {
        warn(WERROR, "Error: no confirmation received for CFG-NAV.");
        return TASK_STOP;
    }

    gpsCheckTime = millis();
    return TASK_RUN;
}

void 
printGPSData (void) 
{
    warn(WLOG, "GPS Data:");
    warnf(WLOG, "Time: %02u:%02u:%02u", 
        gpsData.Hr, gpsData.Min, gpsData.Sec);
    warnf(WLOG, "Lat: %li, Lon: %li", gpsData.Lat, gpsData.Lon);
    warnf(WLOG, "Altitude: %li", gpsData.Alt);
    warnf(WLOG,  "Number of satellites used: %u", gpsData.numSats);
    warnf(WLOG, "Type of lock: %u\n", gpsData.fixType);
}

// Good fix is > 4 sats and fixType = 3 or 4.
void 
checkForLock (void)
{
    if(((int)gpsData.fixType == 3 || 
        (int)gpsData.fixType == 4) && (int)gpsData.numSats > 4
    ) {
        if(gpsLock == false)
            warn(WNOTICE, "*****Lock acquired*****");
        gpsLock         = true;
        lastKnownFix    = gpsData;
    }
    else {
        if(gpsLock == true)
            warn(WNOTICE, "*****Lock lost*****");
        gpsLock = false;
    }
}

// Chops out only the data I want from the UBX sentence
static byte 
gps_parse_pvt (ubx_nav_pvt *pkt)
{
    warnf(WDEBUG, "Decoding packet type [%04x]", pkt->type);
    if (pkt->type != UBX_TYP_NAV_PVT) {
        warnf(WWARN, "Not a NAV-PVT packet");
        return 0;
    }

    gpsData.Valid   = pkt->valid;
    if (gpsData.Valid & UBX_NAVPVT_VALID_TIME) {
        gpsData.Hr      = pkt->hour;
        gpsData.Min     = pkt->min;
        gpsData.Sec     = pkt->sec;
    }

    gpsData.fixType = pkt->fix_type;
    gpsData.numSats = pkt->num_sv;
    gpsData.Lon     = pkt->lon / 100;
    gpsData.Lat     = pkt->lat / 100;
    gpsData.Alt     = pkt->height / 100;

    return 1;
}
