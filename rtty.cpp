
//RTTY Example (Anthony Stirk) trimmed down
//29/01/15
 
#include <stdio.h>
#include <string.h>
#include <util/crc16.h>
 
#include <Arduino.h>

#include "gps.h"
#include "ntx.h"
#include "rtty.h"
#include "warn.h"

#define CALLSIGN    "HABLEEBLEE"

static wchan    rtty_setup          (void);
static wchan    rtty_run            (wchan now);

static uint16_t rtty_checksum       (char *string, byte len);

task rtty_task = {
    .name       = "rtty",
    .when       = TASK_STOP,

    .setup      = rtty_setup,
    .run        = rtty_run,
    .reset      = 0,
};

static wchan
rtty_setup (void) 
{
    warn(WLOG, "RTTY setup");

    ntx_setup();

    return TASK_RUN;
}
 
static wchan 
rtty_run (wchan now) 
{
    static unsigned int id  = 0;

    char        buf[120];
    byte        p;
    uint16_t    checksum;
    gps_data    *g          = &gps_last_fix;

#define FIX(n) (n) / 10000, ((n) < 0 ? -(n) : (n)) % 10000

    p = snprintf_P(buf, sizeof buf,
        sF("$$" CALLSIGN ",%u,%02u:%02u:%02u,%li.%04lu,%li.%04lu,%li"),
        ++id,
        g->hr, g->min, g->sec,
        FIX(g->lat), FIX(g->lon), g->alt
    );
    if (p > sizeof buf) {
        warn(WERROR, "RTTY string too long for buffer");
        goto out;
    }

#undef FIX

    checksum = rtty_checksum(buf, p);
    p += snprintf_P(buf + p, sizeof(buf) - p, sF("*%04x\n"), checksum);
    if (p > sizeof buf) {
        warn(WERROR, "RTTY checksum too long for buffer");
        goto out;
    }

    warnf(WLOG, "RTTY tx [%s]", buf);
    ntx_send((byte *)buf, p);

  out:
    return TASK_SWI(SWI_NTX);
}
 
static uint16_t 
rtty_checksum (char *string, byte len)
{
    size_t i;
    uint16_t crc;
    uint8_t c;
    
    crc = 0xFFFF;
    
    // Calculate checksum ignoring the first two $s
    for (i = 2; i < len; i++) {
        c = string[i];
        crc = _crc_xmodem_update (crc, c);
    }
    
    return crc;
}
