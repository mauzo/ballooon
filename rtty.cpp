//RTTY Example (Anthony Stirk) trimmed down
//29/01/15
 
#include <string.h>
#include <util/crc16.h>
 
#include <Arduino.h>

#include "rtty.h"
#include "warn.h"

#define PIN_RADIO   3
#define PIN_ENABLE  8
#define PWM_HIGH    110
#define PWM_LOW     100

#define RTTY_BAUD   50

static void     rtty_setup          (void);
static void     setup_baudrate      (void);
static void     setup_radio         (void);
static void     rtty_run            (unsigned long now);

static void     rtty_txstring       (char * string);
static void     rtty_txbyte         (char c);
static void     rtty_txbit          (int bit);
static uint16_t gps_CRC16_checksum  (char *string);

static char             datastring[120];
static volatile byte    next_bit = 0;

task rtty_task = {
    .name       = "rtty",
    .when       = TASK_INACTIVE,

    .setup      = rtty_setup,
    .run        = rtty_run,
    .reset      = 0,
};

/* Should be called with interrupts disabled */
static void
setup_baudrate (void)
{
    TCCR1A = 0;     // set entire TCCR1A register to 0
    TCCR1B = 0;     // same for TCCR1B
    
    OCR1A = F_CPU / 1024 / RTTY_BAUD - 1;  // set compare match register to desired timer count:
    
    TCCR1B |= (1 << WGM12);   // turn on CTC (Clear Timer on Compare) mode:
    
    // Set CS10 and CS12 bits on for 1024 pre-scaler
    TCCR1B |= (1 << CS10);
    TCCR1B |= (1 << CS12);
    
    // enable timer compare interrupt:
    TIMSK1 |= (1 << OCIE1A);
}

static void
setup_radio (void)
{
    pinMode(PIN_RADIO, OUTPUT);
    pinMode(PIN_ENABLE, OUTPUT);
    
    // Enable NTX2B (due to FA firmware)
    digitalWrite(PIN_ENABLE, HIGH);
    delay(100);
    digitalWrite(PIN_ENABLE, LOW);
    delay(100);
    digitalWrite(PIN_ENABLE, HIGH);

    // Set PWM freq on timer 2
    TCCR2B = TCCR2B & 0b11111000 | 0b00000001;
}

static void
rtty_setup (void) 
{
    warn(WLOG, "RTTY setup");

    cli();
    setup_baudrate();
    setup_radio();
    sei();

    rtty_task.when = TASK_START;
}
 
static void 
rtty_run (unsigned long now) 
{
    snprintf(datastring,120,"$$HABLEEBLEE,000138,14:00:50,+51.482580670,-003.163666160,000004,+20,-20,05,0196"); // Example datastring
    unsigned int CHECKSUM = gps_CRC16_checksum(datastring); // Calculates the checksum for this datastring
    char checksum_str[6];
    sprintf(checksum_str, "*%04X\n", CHECKSUM);
    strcat(datastring,checksum_str);
    warnf(WLOG, "RTTY tx [%s]", datastring);
    rtty_txstring (datastring);
}
 
static void 
rtty_txstring (char *string)
{ 
    char c;
    
    c = *string++;
    
    while (c != '\0') {
        rtty_txbyte(c);
        c = *string++;
    }
}

static void 
rtty_txbyte (char c)
{
    int i;
 
    rtty_txbit (0); // Start bit
 
    // Send bits for for char LSB first
 
    for (i=0;i<7;i++) { // Change this here 7 or 8 for ASCII-7 / ASCII-8
        if (c & 1)
            rtty_txbit(1);
        else
            rtty_txbit(0);
        c = c >> 1;

        delayMicroseconds(10000); // For 50 Baud uncomment this and the line below.
        delayMicroseconds(10150); // You can't do 20150 it just doesn't work as the
    }
    rtty_txbit (1); // Stop bit
    rtty_txbit (1); // Stop bit
}

 
static void 
rtty_txbit (int bit)
{
    next_bit = bit;
}

ISR(TIMER1_COMPA_vect)
{
    analogWrite(PIN_RADIO, next_bit ? PWM_HIGH : PWM_LOW);
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
