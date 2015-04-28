
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

#define FIX2DEC(n) (n) / 10000, ((n) < 0 ? -(n) : (n)) % 10000

static wchan    rtty_setup          (void);
static wchan    rtty_run            (wchan now);

static uint16_t rtty_checksum       (char *string, byte len);
static byte     rtty_fmt_buf        (char *buf, byte len, gps_data *g);

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
    byte        *buf;
    byte        len;
    gps_data    *g          = &gps_last_fix;

    if (!GPS_FIX_VALID(g)) {
        warn(WLOG, "No valid fix for RTTY to use");
        goto fail;
    }
    if (!(buf = ntx_get_buf(&len))) {
        warn(WWARN, "No NTX buffer available for RTTY send");
        goto fail;
    }
    if (!(len = rtty_fmt_buf((char *)buf, len, g)))
        goto fail;

    /* grr, avr-libc doesn't support %.*s, so we have to rely on the
     * null-termination.
     */
    warnf(WLOG, "RTTY tx [%s]", buf);
    ntx_send(len);

    return TASK_SWI(SWI_NTX);

  fail:
    return TASK_DELAY(2000);
}

static byte
rtty_fmt_buf (char *buf, byte len, gps_data *g)
{
    static unsigned int id  = 0;

    byte        p;
    uint16_t    checksum;

    p = snprintf_P(buf, len,
        sF("$$" CALLSIGN ",%u,%02u:%02u:%02u,%li.%04lu,%li.%04lu,%li"),
        ++id,
        g->hr, g->min, g->sec,
        FIX2DEC(g->lat), FIX2DEC(g->lon), g->alt
    );
    if (p > len) {
        warn(WERROR, "RTTY string too long for buffer");
        return 0;
    }

    checksum = rtty_checksum(buf, p);
    p += snprintf_P(buf + p, len - p, sF("*%04x\n"), checksum);
    if (p > len) {
        warn(WERROR, "RTTY checksum too long for buffer");
        return 0;
    }

    return p;
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
