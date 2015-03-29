/* HardwareSerial.h
 * amd64 substitute for the HardwareSerial class.
 */

#include <inttypes.h>
#include <stdio.h>

#include <Print.h>
#include <Stream.h>

#warning My HardwareSerial.h

class HardwareSerial : public Stream {
public:
    HardwareSerial () { }
    void begin (unsigned long) { setlinebuf(stdout); }
    virtual size_t write (uint8_t c) { putc(c, stdout); return 1; }
    virtual int available () { return 0; }
    virtual int read () { return -1; }
    virtual int peek () { return -1; }
    virtual void flush () { }
    using Print::write;
};

static HardwareSerial Serial = HardwareSerial();
