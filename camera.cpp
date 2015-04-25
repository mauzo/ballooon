//First attempt at switching camera on with BJT

#include <Arduino.h>

#include "camera.h"
#include "task.h"
#include "warn.h"

static wchan  cam_setup	(void);
static wchan  cam_shoot	(wchan now);
static void   cam_power	(void);

task cam_task = {
    .name   = "camera",
    .when   = TASK_STOP,

    .setup  = cam_setup,
    .run    = cam_shoot,
    .reset  = 0,
};

#define CAM_START   0
#define CAM_FOCUS   1
#define CAM_SHUTTER 2
#define CAM_STORE   3
#define CAM_FINISH  4

static wchan 
cam_setup (void)
{
    pinMode(POWERPIN,OUTPUT);
    digitalWrite(POWERPIN,LOW);
    pinMode(FOCUSPIN,OUTPUT);
    digitalWrite(FOCUSPIN,LOW);
    pinMode(SHUTTER,OUTPUT);
    digitalWrite(SHUTTER,LOW);

    return TASK_RUN;
}

static wchan 
cam_shoot (wchan now)
{
    static byte cam_state   = CAM_START;
    static byte cam_shot = 1;

    /* Don't use the time passed in, get our own time, because we want a
     * fixed-length delay from changing the pin state, not a 'called
     * once every N seconds' delay. The timing will not be very precise;
     * if that matters, we'll have to set .when for somewhat less than
     * the delay we need, and busy-wait the last bit here. */
    switch (cam_state++) {
    case CAM_START:
        warn(WLOG, "cam: taking a picture");
        cam_power();
        return TASK_DELAY(3000); //3 secs to allow lens to extend and power on
    case CAM_FOCUS:
        warn(WDEBUG, "CAM_FOCUS");
        digitalWrite(FOCUSPIN,HIGH);
        return TASK_DELAY(2000); //2 secs to allow focus
    case CAM_SHUTTER:
        warn(WDEBUG, "CAM_SHUTTER");
        digitalWrite(SHUTTER,HIGH);
        return TASK_DELAY(200); //Brief hold of button XXX how long?
    case CAM_STORE:
        warn(WDEBUG, "CAM_STORE");
        digitalWrite(SHUTTER,LOW);
        digitalWrite(FOCUSPIN,LOW);
        if(cam_shot++ < 5)
            cam_state   = CAM_FOCUS;
        return TASK_DELAY(3000); //3 secs to store image on card
    case CAM_FINISH:
        warn(WDEBUG, "CAM_FINISH");
        cam_power();
        cam_shot        = 1;
        cam_state       = CAM_START;
        /* XXX ~5 mins until next shots? */
        return TASK_DELAY(300000);
    }

    warn(WERROR, "Camera task: bad state");
    return TASK_STOP;
}

static void 
cam_power()
{
    digitalWrite(POWERPIN,HIGH);
    delay(200); //XXX how long a hold - experiment!
    digitalWrite(POWERPIN,LOW);
}
