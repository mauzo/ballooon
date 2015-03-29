/* gps.cpp
 * Functions for talking to the GPS module.
 */

#include <Arduino.h>

#include "ubx.h"
#include "ubx_pkt.h"
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

static void     gps_setup   (void);
static void     gps_run     (unsigned long now);
static long     join4Bytes  (byte *data);

task gps_task = {
    .name   = "GPS",
    .when   = 0,

    .setup  = gps_setup,
    .run    = gps_run,
    .reset  = 0,
};

static void
gps_run (unsigned long now)
{
    warn(WDEBUG, "Requesting NAV-PVT.");
    if(!sendUBX(GPS_ADDR, reqNAV_PVT, sizeof(reqNAV_PVT)))
        warn(WERROR, "Error sending NAV-PVT");
    getGPSData(GPS_ADDR);
    checkForLock();
    printGPSData();

    gps_task.when = now + 10000;
}

static void
gps_setup (void)
{
    warn(WNOTICE, "Setting IO protocols.");
    if(!sendUBX(GPS_ADDR, setIOtoUBX, sizeof(setIOtoUBX)))
        warn(WERROR, "Error sending CFG-PRT.");
    
    warn(WDEBUG, "Awaiting confirmation.");
    if(!getUBX_ACK(GPS_ADDR, setIOtoUBX))
        warn(WERROR, "Error: no confirmation received for CFG-PRT.");
    
    warn(WNOTICE, "Setting flight mode (1g).");
    if(!sendUBX(GPS_ADDR, airborne1g, sizeof(airborne1g)))
        warn(WERROR, "Error sending CFG-NAV.");
    
    warn(WNOTICE, "Awaiting confirmation.");
    if(!getUBX_ACK(GPS_ADDR, airborne1g))
        warn(WERROR, "Error: no confirmation received for CFG-NAV.");

    gpsCheckTime = millis();
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
void 
parseUBX (void)
{
    warnf(WDEBUG, "Decoding packet class [%x] id [%x]", UBXclass, UBXid);
    if(UBXclass == 0x01 && UBXid == 0x07) {
        gpsData.Hr      = UBXbuffer[8];
        gpsData.Min     = UBXbuffer[9];
        gpsData.Sec     = UBXbuffer[10];
        gpsData.Valid   = UBXbuffer[11];
        gpsData.fixType = UBXbuffer[20];
        gpsData.numSats = UBXbuffer[23];
        gpsData.Lon     = join4Bytes(&UBXbuffer[24]) / 100;
        gpsData.Lat     = join4Bytes(&UBXbuffer[28]) / 100;
        gpsData.Alt     = join4Bytes(&UBXbuffer[36]) / 100;
    }
}

// Joins 4 bytes into one long. Pointers!
static long 
join4Bytes (byte *data)
{
    union long_union {
        int32_t dword;
        uint8_t  byte[4];
    } longUnion;

    longUnion.byte[0] = *data;
    longUnion.byte[1] = *(data+1);
    longUnion.byte[2] = *(data+2);
    longUnion.byte[3] = *(data+3);

    return(longUnion.dword);
}

