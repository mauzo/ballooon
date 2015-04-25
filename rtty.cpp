
//RTTY Example (Anthony Stirk) trimmed down
//29/01/15
 
#include <stdio.h>
#include <string.h>
#include <util/crc16.h>
 
#include <Arduino.h>

#include "ntx.h"
#include "rtty.h"
#include "warn.h"

static wchan    rtty_setup          (void);
static wchan    rtty_run            (wchan now);

static uint16_t gps_CRC16_checksum  (char *string);

static char             datastring[120];

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
    unsigned int    checksum;
    char            checksum_str[7];


    snprintf(datastring,120,"$$HABLEEBLEE");
    checksum = gps_CRC16_checksum(datastring);
    snprintf(checksum_str, sizeof checksum_str, "*%04x\n", checksum);
    strcat(datastring,checksum_str);
    warnf(WLOG, "RTTY tx [%s]", datastring);
    ntx_send((byte*)datastring, 120);
    warn(WDEBUG, "RTTY done send");

    return TASK_SWI(SWI_NTX);
}
 
static uint16_t 
gps_CRC16_checksum (char *string)
{
    size_t i;
    uint16_t crc;
    uint8_t c;
    
    crc = 0xFFFF;
    
    // Calculate checksum ignoring the first two $s
    for (i = 2; i < strlen(string); i++) {
        c = string[i];
        crc = _crc_xmodem_update (crc, c);
    }
    
    return crc;
}
