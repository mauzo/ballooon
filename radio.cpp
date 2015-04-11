//First attempt at switching camera on with BJT

#include <Arduino.h>

#include "radio.h"
#include "task.h"
#include "warn.h"

static void   radio_setup	(void);
static void   radio_tx  	(unsigned long now);

task radio_task = {
    .name   = "radio",
    .when   = TASK_INACTIVE,
    .setup  = radio_setup,
    .run    = radio_tx,
    .reset  = 0,
};

static const byte rtty_string[] = "Hello world!\n";
static const byte *rtty_ptr     = rtty_string;
static byte rtty_byte           = *rtty_ptr;
static byte rtty_bit            = 0;
static volatile unsigned long rtty_sent  = 0;

static void 
radio_setup (void)
{
    warn(WDEBUG, "Radio setup");
    pinMode(RADIO_PIN, OUTPUT);
    warn(WDEBUG, "Set radio pin to output");
    warnf(WDEBUG, "Radio pin [%u] is using timer [%u]",
	(unsigned)RADIO_PIN, (unsigned)digitalPinToTimer(RADIO_PIN));
    
    pinMode(ENABLE_PIN, OUTPUT);
    digitalWrite(ENABLE_PIN, HIGH);
    delay(100);
    digitalWrite(ENABLE_PIN, LOW);
    delay(100);
    digitalWrite(ENABLE_PIN, HIGH);  

    warn(WDEBUG, "Radio: about to cli");
    cli();
    
    TCCR1A = 0;
    TCCR1B = 0;
    
    //warn(WDEBUG, "Set TCCR1*");

    // set compare match register to desired timer count:
    OCR1A = F_CPU / 1024 / RTTY_BAUD - 1;
    //warn(WDEBUG, "Set OCRA1");

    // turn on CTC (Clear Timer on Compare) mode:
    TCCR1B |= (1 << WGM12);
    //warn(WDEBUG, "Set CTC");

    // Set CS10 and CS12 bits on for 1024 pre-scaler
    TCCR1B |= (1 << CS10);
    TCCR1B |= (1 << CS12);
    //warn(WDEBUG, "Set 1024 prescaler");
    
    // enable timer compare interrupt:
    TIMSK1 |= (1 << OCIE1A);
    //warn(WDEBUG, "Enable interrupt");
    //return;
    
    // Set PWM frequency: clk_IO, no prescale
    TCCR2B = TCCR2B & 0b11111000 | 0b00000001;
    
    // Set PWM output
    TCCR2A = TCCR2A & 0b11001111 | 0b00110000;

    sei();
    warn(WDEBUG, "Radio: done sei");
    
    radio_task.when = TASK_START;
}

static void 
radio_tx (unsigned long now)
{
    unsigned long c;
    cli(); c = rtty_sent; sei();
    warnf(WDEBUG, "Radio: sent [%lu] bits", c);
    radio_task.when = now + 2000;
    /* Nothing; we do everything in the ISR */
}

ISR(TIMER1_COMPA_vect)
{
    cli(); 
    rtty_sent++;
    //warnf(WDEBUG, "RTTY: bit %u", rtty_byte & 1);
    //if ((rtty_sent / 50) % 2)
    //    analogWrite(RADIO_PIN, 110);
    //else
    //    analogWrite(RADIO_PIN, 100);
    analogWrite(RADIO_PIN, rtty_sent);
    //return;

    rtty_byte = rtty_byte >> 1;
    if (++rtty_bit > 8) {
        rtty_bit = 0;
        rtty_byte = *rtty_ptr++;
        if (!*rtty_ptr)
            rtty_ptr = rtty_string;
    }
    sei();
}
