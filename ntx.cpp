/* ntx.cpp
 * Transmit over the NTX radio module.
 */

#include <Arduino.h>

#define PIN_RADIO   3
#define PIN_ENABLE  8
#define PWM_HIGH    110
#define PWM_LOW     100

#define NTX_BAUD    50

/* Timer prescalar values (in TCCxB) */
#define PSC_MASK    0xf8
#define PSC_STOP    0
#define PSC_FULL    1
#define PSC_8       2
#define PSC_32      3
#define PSC_64      4
#define PSC_128     5
#define PSC_256     6
#define PSC_1024    7

static volatile byte    next_bit = 0;

static void     setup_radio         (void);
static void     setup_timer         (void);
static void     timer_disable       (void);
static void     timer_enable        (void);

void
ntx_setup (void)
{
    cli();
    setup_radio();
    setup_timer();
    sei();
}

/* Set up timer 1 (the 16-bit timer) to tick at the baud rate.
 * Should be called with interrupts disabled. */
static void
setup_timer (void)
{
    timer_disable();

    TCCR1A = 0;
    TCCR1B = 0;

    /* set prescalar */
    TCCR1B = TCCR2B & PSC_MASK | PSC_1024;
    
    /* set up CTC mode */
    OCR1A = F_CPU / 1024 / NTX_BAUD - 1;
    TCCR1B |= (1 << WGM12);
}

static void
timer_enable (void)
{
    /* enable timer interrupt */
    TIMSK1 |= (1 << OCIE1A);
}

static void
timer_disable (void)
{
    /* disable timer interrupt */
    TIMSK1 &= ~(1 << OCIE1A);
}

/* Should be called with interrupts disabled */
static void
setup_radio (void)
{
    pinMode(PIN_RADIO, OUTPUT);
    pinMode(PIN_ENABLE, OUTPUT);
    
    /* Set radio to analogue mode */
    digitalWrite(PIN_ENABLE, HIGH);
    delay(100);
    digitalWrite(PIN_ENABLE, LOW);
    delay(100);
    digitalWrite(PIN_ENABLE, HIGH);

    /* Set PWM timer to no prescaling */
    TCCR2B = TCCR2B & PSC_MASK | PSC_FULL;
}

void 
ntx_txbit (byte bit)
{
    next_bit = bit;
    timer_enable();
}

ISR(TIMER1_COMPA_vect)
{
    analogWrite(PIN_RADIO, next_bit ? PWM_HIGH : PWM_LOW);
}

