
//RTTY Example (Anthony Stirk) trimmed down
//29/01/15
 
#include <string.h>
#include <util/crc16.h>
 
#include <Arduino.h>

#include "ntx.h"
#include "rtty.h"
#include "warn.h"

static void     rtty_setup          (void);
static void     rtty_run            (unsigned long now);

static uint16_t gps_CRC16_checksum  (char *string);

static char             datastring[120];

task rtty_task = {
    .name       = "rtty",
    .when       = TASK_INACTIVE,

    .setup      = rtty_setup,
    .run        = rtty_run,
    .reset      = 0,
};

static void
rtty_setup (void) 
{
    warn(WLOG, "RTTY setup");

    ntx_setup();

    rtty_task.when = TASK_START;
}
 
static void 
rtty_run (unsigned long now) 
{
    unsigned int    checksum;
    char            checksum_str[6];

    snprintf(datastring,120,"$$HBLEE");
    checksum = gps_CRC16_checksum(datastring);
    sprintf(checksum_str, "*%04X\n", checksum);
    strcat(datastring,checksum_str);
    warnf(WLOG, "RTTY tx [%s]", datastring);
    ntx_send((byte*)datastring, strlen(datastring));

    rtty_task.when = now + 20000;
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
