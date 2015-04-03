/* ubx.cpp
 * Functions for talking the UBX protocol.
 */

#include <Arduino.h>
#include <Wire.h>

#include "ubx.h"
#include "warn.h"

static byte     ubx_buf[UBX_BUFSIZ];
static byte     ubx_dropped;

static byte     ubx_check_sum       (ubx_pkt *pkt);
static void     ubx_drop            (ubx_addr adr, uint16_t len);
static byte     ubx_getb            (ubx_addr adr);
static void     ubx_get_buf         (ubx_addr adr, byte *buf, byte len);
static byte     ubx_get_sync        (ubx_addr adr);

static unsigned int
ubx_avail(ubx_addr adr)
{
    unsigned int bytes_avail;
  
    Wire.beginTransmission(adr);
    Wire.write(0xFD);
    /* it's important not to send a STOP here */
    Wire.endTransmission(0);
    
    Wire.requestFrom(adr, 2);

    bytes_avail = (uint16_t)Wire.read() << 8;
    bytes_avail |= (uint16_t)Wire.read();
    
    //warnf(WDEBUG, "ubx_avail returns %u", bytes_avail);
    return bytes_avail;
}

static byte
ubx_check_sum (ubx_pkt *pkt)
{
    byte    *p  = (byte *)pkt;
    int     l   = pkt->len + UBX_HEADSIZ;
    byte    a = 0, b = 0;

    while (l) {
        a += *p;
        b += a;
        p++; l--;
    }

    if (p[0] == a && p[1] == b)
        return 1;

    warnf(WWARN, "UBX packet failed checksum [%02x%02x] vs [%02x%02x]",
        p[0], p[1], a, b);
    ubx_drop(0, 0);
    return 0;
}

static void
ubx_drop (ubx_addr adr, uint16_t len)
{
    /* XXX check for too many drops */
    warn(WWARN, "Dropping UBX packet");
    ubx_dropped++;

    while (len--)
        ubx_getb(adr);
}

static byte
ubx_getb (ubx_addr adr)
{
    if (!Wire.available())
        Wire.requestFrom(adr, BUFFER_LENGTH);

    return Wire.read();
}

static void
ubx_get_buf (ubx_addr adr, byte *buf, byte len)
{
    while (len--)
        *buf++ = ubx_getb(adr);
}

static byte
ubx_get_sync (ubx_addr adr)
{
    byte    b;
    byte    sync       = 0;
    int     timeout    = 300;

    while (1) {
        /* XXX I don't like the fact that we only have one buffer,
         * inside Wire, for communication with all devices on the bus.
         * It's not a problem atm but would become one if we had more
         * than one device.
         */
        if(Wire.available() || ubx_avail(adr)) {
            b = ubx_getb(adr);
        }
        else {
            if (!timeout--) {
                warn(WWARN, "Timeout looking for UBX sync");
                return 0;
            }
            delay(10);
            continue;
        }

        switch (b) {
        case 0xff:
            warn(WDEBUG, "Got 0xff looking for sync");
            sync = 0;
            break;
        case 0xb5:
            sync = 1;
            break;
        case 0x62:
            if (sync)
                return 1;
            /* fall through */
        default:
            warnf(WWARN, "Unexpected sync byte 0x%02x", b);
            sync = 0;
            break;
        }
    }
}

ubx_pkt *
ubx_read (ubx_addr adr)
{
    ubx_pkt     *pkt    = (ubx_pkt *)ubx_buf;
    uint16_t    len;
    
    /* This should not be necessary now... */
#if 0
    while(Wire.available())
        Wire.read();
#endif
    
    if (!ubx_get_sync(adr))
        return NULL;

    ubx_get_buf(adr, ubx_buf, UBX_HEADSIZ);
    len = pkt->len + 2;

    if (len + UBX_HEADSIZ > sizeof(ubx_buf)) {
        warnf(WWARN, "UBX packet of type [%04x] too long (%u)", 
            pkt->type, len);
        ubx_drop(adr, len);
        return NULL;
    }

    ubx_get_buf(adr, pkt->dat, len);
    if (!ubx_check_sum(pkt))
        return NULL;

    return pkt;
}

// Send a byte array of UBX protocol to the GPS
boolean 
sendUBX (ubx_addr adr, byte *msg, byte len)
{
    byte    i;
    boolean result;

    // Start I2C transmission
    Wire.beginTransmission(adr);

    /* print a stamp for the debugging output */
    warn_stamp(WDUMP);

    //Write one byte at a time
    for(i=0; i<len; i++) {
        // Wire library bug. Detect 32 bytes (defined in Wire.h) and
        // start again
        if(i > 0 && i % BUFFER_LENGTH == 0) {
            if(Wire.endTransmission() != 0) {
                warn(WERROR, "Error in Wire.endTransmission at buffer=max.");
                return result;
            }
            
            Wire.beginTransmission(adr);
        }

        warnxf(WDUMP, " %02x", msg[i]);
        if(Wire.write(msg[i]) != 1) {
            warn(WERROR, "Error in Wire.write.");
            return result;
        }
    }
    warn_nl(WDUMP);

    if(Wire.endTransmission() != 0) {
        warn(WERROR, "Error in Wire.endTransmission.");
        return result;
    }

    warn(WDEBUG, "Success!");
    return true;
}

// Calculate expected UBX ACK packet and parse UBX response from GPS
boolean 
getUBX_ACK (ubx_addr adr, byte *msg) 
{
    byte    readByte;
    byte    ackByteID       = 0;
    byte    ackPacket[10];
    byte    i;
    unsigned long startTime = millis();

    warn(WDEBUG, "Reading ACK response");
    
    // Construct the expected ACK packet    
    ackPacket[0] = 0xB5;	// header
    ackPacket[1] = 0x62;	// header
    ackPacket[2] = 0x05;	// class
    ackPacket[3] = 0x01;	// id
    ackPacket[4] = 0x02;	// length1
    ackPacket[5] = 0x00;        // length2
    ackPacket[6] = msg[2];	// ACK class
    ackPacket[7] = msg[3];	// ACK id
    ackPacket[8] = 0;	        // CK_A
    ackPacket[9] = 0;	        // CK_B
    
    // Calculate the first part of the checksum
    for (i=2; i<8; i++) {
        ackPacket[8] = ackPacket[8] + ackPacket[i];
        ackPacket[9] = ackPacket[9] + ackPacket[8];
    }
    
    /* timestamp the hex dump */
    warn_stamp(WDUMP);

    while (1) {
        // Timeout if no valid response in 3 seconds
        if (millis() - startTime > 3000) { 
            warn(WERROR, "Timed out reading UBX ACK");
            return false;
        }
        
        // Make sure data is available to read
        Wire.requestFrom(adr, BUFFER_LENGTH);
        
        while (Wire.available()) {
            readByte = Wire.read();

            // The UBX device sends 0xff when it has nothing left to
            // send, so ignore that.
            if(readByte != 0xFF) {
                // Check that bytes arrive in sequence as per expected
                // ACK packet
                if (readByte == ackPacket[ackByteID]) { 
                    ackByteID++;
                    warnxf(WDUMP, " %02x", readByte);
                    
                    // Test for success
                    if (ackByteID > 9) {
                        // All packets in order!
                        warn_nl(WDUMP);
                        warn(WDEBUG, "Successfully read UBX ACK");
                        return true;
                    }
                }
                else {
                    // Reset and look again, invalid order
                    ackByteID = 0;	
                }
            } 
        }
    }
}

void
ubx_setup (void)
{
    Wire.begin(); // Start I2C link to GPS
}


