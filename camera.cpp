//First attempt at switching camera on with BJT

#include <Arduino.h>

#include "camera.h"
#include "task.h"
#include "warn.h"

static wchan    cam_setup       (void);
static wchan    cam_shoot       (wchan now);
static byte     cam_check_power (byte expect);
static void     cam_power       (void);

task cam_task = {
    .name   = "camera",
    .when   = TASK_STOP,

    .setup  = cam_setup,
    .run    = cam_shoot,
    .reset  = 0,
};

#define PIN_DETECT      3
#define PIN_POWER	4
#define PIN_FOCUS	5
#define PIN_SHUTTER	6

#define CAM_START       0
#define CAM_FOCUS       1
#define CAM_SHUTTER     2
#define CAM_STORE       3
#define CAM_POWEROFF    4
#define CAM_FINISH      5

static wchan 
cam_setup (void)
{
    pinMode(PIN_DETECT, INPUT);
    pinMode(PIN_POWER,OUTPUT);
    digitalWrite(PIN_POWER,LOW);
    pinMode(PIN_FOCUS,OUTPUT);
    digitalWrite(PIN_FOCUS,LOW);
    pinMode(PIN_SHUTTER,OUTPUT);
    digitalWrite(PIN_SHUTTER,LOW);

    return TASK_RUN;
}

static wchan 
cam_shoot (wchan now)
{
    static byte cam_state   = CAM_START;
    static byte cam_shot    = 1;

    switch (cam_state++) {
    case CAM_START:
        warn(WLOG, "cam: taking a picture");
        cam_shot        = 1;
        cam_power();
        return TASK_DELAY(3000); //3 secs to allow lens to extend and power on

    case CAM_FOCUS:
        warn(WDEBUG, "CAM_FOCUS");
        if (!cam_check_power(1)) {
            cam_state   = CAM_START;
            return TASK_RUN;
        }
        digitalWrite(PIN_FOCUS, HIGH);
        return TASK_DELAY(2000); //2 secs to allow focus

    case CAM_SHUTTER:
        warn(WDEBUG, "CAM_SHUTTER");
        digitalWrite(PIN_SHUTTER, HIGH);
        return TASK_DELAY(200); //Brief hold of button XXX how long?

    case CAM_STORE:
        warn(WDEBUG, "CAM_STORE");
        digitalWrite(PIN_SHUTTER, LOW);
        digitalWrite(PIN_FOCUS, LOW);
        if(cam_shot++ < 5)
            cam_state   = CAM_FOCUS;
        return TASK_DELAY(3000); //3 secs to store image on card

    case CAM_POWEROFF:
        warn(WDEBUG, "CAM_POWEROFF");
        cam_power();
        return TASK_DELAY(3000); // 3s to complete powerdown

    case CAM_FINISH:
        warn(WDEBUG, "CAM_FINISH");
        if (!cam_check_power(0)) {
            cam_state   = CAM_POWEROFF;
            return TASK_RUN;
        }
        cam_state       = CAM_START;
        /* XXX ~5 mins until next shots? */
        return TASK_DELAY(5*60*1000L);

    default:
        warn(WERROR, "Camera task: bad state");
        cam_check_power(0);
        cam_state       = CAM_START;
        return TASK_DELAY(3*60*1000L);
    }
}

static byte
cam_check_power (byte expect)
{
    byte s;

    s = digitalRead(PIN_DETECT);
    if (s == expect)
        return 1;

    warnf(WWARN, "Camera powered [%S] when it should be [%S]",
        (s ? sF("on") : sF("off")), 
        (expect ? sF("on") : sF("off"))
    );
    cam_power();
    delay(300);

    s = digitalRead(PIN_DETECT);
    if (s == expect)
        return 1;

    return 0;
}

static void 
cam_power (void)
{
    digitalWrite(PIN_POWER, HIGH);
    delay(200); //XXX how long a hold - experiment!
    digitalWrite(PIN_POWER, LOW);
}
