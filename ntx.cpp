/* ntx.cpp
 * Transmit over the NTX radio module.
 */

#include <Arduino.h>

#include "atomic.h"
#include "ntx.h"
#include "warn.h"

#if defined(BOARD_UNO)
#  define PIN_RADIO     3
#  define PIN_ENABLE    8
#elif defined(BOARD_MEGA) || defined(BOARD_MEGA2560)
#  define PIN_RADIO     11
#  define PIN_ENABLE    8
#else
#  error "I don't know which pins to use on this board"
#endif

#define PWM_HIGH    110
#define PWM_LOW     100

#define NTX_BAUD    50
#define NTX_NSTART  1
#define NTX_NDATA   7
#define NTX_NSTOP   2

#define STATE_NONE  0
#define STATE_START 1
#define STATE_DATA  2
#define STATE_STOP  3

/* Timer prescalar values (in TCCxB) */
#define PSC_MASK    0xf8
#define PSC_STOP    0
#define PSC_FULL    1
#define PSC_8       2
#define PSC_64      3
#define PSC_256     4
#define PSC_1024    5
#define PSC_EXTF    6
#define PSC_EXTR    7

static byte             ntx_buf[NTX_BUFSIZ];
static byte             ntx_len     = 0;
static volatile byte    ntx_ix      = 0;
static volatile byte    ntx_state   = STATE_NONE;
static byte             ntx_bit     = 0;

#ifdef NTX_DEBUG
/* Each byte of this array records one bit which has been sent. The byte
 * is in the format 000boonn, where b is the bit that was sent, oo is
 * the old state, and nn is the new state.
 */
static volatile byte    ntx_sent[NTX_BUFSIZ];
static volatile byte    ntx_nsent   = 0;
static volatile byte    ntx_isrs    = 0;

static wchan    ntx_run             (wchan now);
#endif

static void     setup_radio         (void);
static void     setup_timer         (void);
static void     timer_disable       (void);
static void     timer_enable        (void);

#ifdef NTX_DEBUG
task ntx_task = {
    .name       = "NTX",
    .when       = TASK_STOP,

    .setup      = 0,
    .run        = ntx_run,
    .reset      = 0,
};
#endif

void
ntx_setup (void)
{
    CRIT_START {
        setup_radio();
        setup_timer();
    } CRIT_END;
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
    TCCR1B = TCCR1B & PSC_MASK | PSC_1024;
    
    /* set up CTC mode */
    OCR1A = F_CPU / 1024 / NTX_BAUD - 1;
    TCCR1B |= (1 << WGM12);
}

/* XXX These are nonatomic: the compiler can't use [sc]bi since these
 * registers are outside IO space. This, presumably, means games with
 * cli and SREG?
 */
static void
timer_enable (void)
{
    CRIT_START {
        /* enable timer interrupt */
        TIMSK1 |= (1 << OCIE1A);
    } CRIT_END;
}

static void
timer_disable (void)
{
    CRIT_START {
        /* disable timer interrupt */
        TIMSK1 &= ~(1 << OCIE1A);
    } CRIT_END;
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

#ifdef NTX_DEBUG
wchan
ntx_run (wchan now)
{
    static char     states[][6] = { "none", "start", "data", "stop" };
    byte            i;

    timer_disable();

    warnf(WDUMP, "NTX sent [%u] bits in [%u] isrs (%s/%u)", 
        (unsigned)ntx_nsent, ntx_isrs,
        states[ntx_state], (unsigned)ntx_bit);
    for (i = 0; i < ntx_nsent; i++) {
        warnf(WDUMP, "NTX sent bit [%u]: [%s] -> [%s]",
            (ntx_sent[i] & 0x10) >> 4,
            states[(ntx_sent[i] & 0xc) >> 2],
            states[ntx_sent[i] & 0x3]);
    }
    ntx_nsent = 0;

    if (ntx_state != STATE_NONE) {
        warn(WDEBUG, "Re-enabling NTX timer");
        timer_enable();
    }

    return TASK_TIME(now, 2000);
}
#endif

byte
ntx_send (byte *buf, byte len)
{
    if (ntx_state != STATE_NONE)
        return 0;

    timer_disable();

    len = min(len, NTX_BUFSIZ);
    memcpy(ntx_buf, buf, len);
    warnf(WDEBUG, "Copied [%u] bytes into NTX buffer", (unsigned)len);

    ntx_ix      = 0;
    ntx_len     = len;
    ntx_state   = STATE_START;
    warn(WDEBUG, "Set ntx_state");
    timer_enable();

    return len;
}

ISR(TIMER1_COMPA_vect)
{
    byte    b;

#ifdef NTX_DEBUG
    ntx_isrs++;
    ntx_sent[ntx_nsent] = ntx_state << 2;
#endif

    /* XXX parity? */
    switch (ntx_state) {
    case STATE_NONE:
        timer_disable();
        return;
    case STATE_START:
        b = 0;
        if (++ntx_bit == NTX_NSTART) {
            ntx_bit     = 0;
            ntx_state   = STATE_DATA;
        }
        break;
    case STATE_DATA:
        b = ntx_buf[ntx_ix] & (1 << ntx_bit);
        if (++ntx_bit == NTX_NDATA) {
            ntx_bit     = 0;
            ntx_state   = STATE_STOP;
        }
        break;
    case STATE_STOP:
        b = 1;
        if (++ntx_bit == NTX_NSTOP) {
            ntx_bit     = 0;
            if (ntx_ix++ > ntx_len) {
                ntx_state   = STATE_NONE;
                swi(SWI_NTX);
            }
            else {
                ntx_state   = STATE_START;
            }
        }
        break;
    default:
        /* XXX panic */
        warnf(WPANIC, "Bad ntx_state value [%u]", (unsigned)ntx_state);
        return;
    }

    analogWrite(PIN_RADIO, b ? PWM_HIGH : PWM_LOW);
#ifdef NTX_DEBUG
    b = !!b;
    ntx_sent[ntx_nsent++] |= (b << 4) | ntx_state;
#endif
}

