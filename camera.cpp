//First attempt at switching camera on with BJT

#include <Arduino.h>

#include "camera.h"
#include "task.h"
#include "warn.h"

static void   cam_setup	(void);
static void   cam_shoot	(unsigned long now);
static void   cam_power	(void);

task cam_task = {
    .name   = "camera",
    .when   = 0,

    .setup  = cam_setup,
    .run    = cam_shoot,
    .reset  = 0,
};

#define CAM_START   0
#define CAM_FOCUS   1
#define CAM_SHUTTER 2
#define CAM_STORE   3
#define CAM_FINISH  4

static byte cam_state   = CAM_START;
static byte cam_shot = 1;

static void 
cam_setup (void)
{
    pinMode(POWERPIN,OUTPUT);
    digitalWrite(POWERPIN,LOW);
    pinMode(FOCUSPIN,OUTPUT);
    digitalWrite(FOCUSPIN,LOW);
    pinMode(SHUTTER,OUTPUT);
    digitalWrite(SHUTTER,LOW);
}

static void 
cam_shoot (unsigned long now)
{
    /* Don't use the time passed in, get our own time, because we want a
     * fixed-length delay from changing the pin state, not a 'called
     * once every N seconds' delay. The timing will not be very precise;
     * if that matters, we'll have to set .when for somewhat less than
     * the delay we need, and busy-wait the last bit here. */
    switch (cam_state++) {
    case CAM_START:
        cam_power();
        cam_task.when = millis() + 3000; //3 secs to allow lens to extend and power on
        break;
    case CAM_FOCUS:
        digitalWrite(FOCUSPIN,HIGH);
        cam_task.when = millis() + 2000; //2 secs to allow focus
        break;
    case CAM_SHUTTER:
        digitalWrite(SHUTTER,HIGH);
        cam_task.when = millis() + 200; //Brief hold of button XXX how long?
        break;
    case CAM_STORE:
        digitalWrite(SHUTTER,LOW);
        digitalWrite(FOCUSPIN,LOW);
        if(cam_shot<5)
            cam_state -= 3; //Go back to CAM_FOCUS to take the next pic
        cam_task.when = millis() + 3000; //3 secs to store image on card
        break;
    case CAM_FINISH:
        cam_shot=1;
        cam_power();
        cam_state       = CAM_START;
        /* XXX ~5 mins until next shots? */
        cam_task.when   = millis() + 300000;
        break;
    }
}

static void 
cam_power()
{
    digitalWrite(POWERPIN,HIGH);
    delay(200); //XXX how long a hold - experiment!
    digitalWrite(POWERPIN,LOW);
}
