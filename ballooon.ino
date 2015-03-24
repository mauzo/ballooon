/* main.ino
 * Main loop.
 * vi:set syn=cpp:
 */
 
#include "LowPower.h"

void setup()
{
    Serial.begin(9600);
}

#define BEGIN_SLEEP(m) \
    do { \
        Serial.println(m); \
        now = millis(); \
        when = millis() + 20000; \
    } while (0)

#define END_SLEEP \
    do { \
        Serial.print("Slept for ["); \
        Serial.print(millis() - now); \
        Serial.println("]ms, according to millis()"); \
    } while (0)

void loop()
{
    unsigned long now, when;
    byte i;

    BEGIN_SLEEP("Busy-waiting for 20s");
    while (millis() < when) ;
    END_SLEEP;

    BEGIN_SLEEP("Idle for 20s, 250ms at a time, periphs on");
    while (millis() < when)
        LowPower.idle(SLEEP_250MS, ADC_ON, TIMER2_ON, TIMER1_ON,
            TIMER0_ON, SPI_ON, USART0_ON, TWI_ON);
    END_SLEEP;

    BEGIN_SLEEP("Idle for 20s, 4s at a time, periphs on");
    while (millis() < when)
        LowPower.idle(SLEEP_4S, ADC_ON, TIMER2_ON, TIMER1_ON,
            TIMER0_ON, SPI_ON, USART0_ON, TWI_ON);
    END_SLEEP;

    BEGIN_SLEEP("Idle for 20s, 250ms at a time, timer0 on");
    while (millis() < when)
        LowPower.idle(SLEEP_250MS, ADC_OFF, TIMER2_OFF, TIMER1_OFF,
            TIMER0_ON, SPI_OFF, USART0_OFF, TWI_OFF);
    END_SLEEP;

    BEGIN_SLEEP("Idle for 20s, 4s at a time, timer0 on");
    while (millis() < when)
        LowPower.idle(SLEEP_4S, ADC_OFF, TIMER2_OFF, TIMER1_OFF,
            TIMER0_ON, SPI_OFF, USART0_OFF, TWI_OFF);
    END_SLEEP;

    BEGIN_SLEEP("Idle for ~20s, 4s at a time, timer0 off");
    for (i = 0; i < 5; i++)
        LowPower.idle(SLEEP_4S, ADC_OFF, TIMER2_OFF, TIMER1_OFF,
            TIMER0_OFF, SPI_OFF, USART0_OFF, TWI_OFF);
    END_SLEEP;
    
    BEGIN_SLEEP("Standby for ~20s, 4s at a time");
    for (i = 0; i < 5; i++)
        LowPower.powerStandby(SLEEP_4S, ADC_OFF, BOD_ON);
    END_SLEEP;
}
