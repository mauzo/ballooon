//First attempt at switching camera on with BJT

#include <Arduino.h>

#include "camera.h"
#include "task.h"
#include "warn.h"

static wchan    cam_setup       (void);
static wchan    cam_shoot       (wchan now);
static byte     cam_check_power ();
static void     cam_power       (void);

task cam_task = {
    .name   = "camera",
    .when   = TASK_STOP,

    .setup  = cam_setup,
    .run    = cam_shoot,
    .reset  = 0,
};

#if defined(BOARD_UNO)
#  define PIN_DETECT    4
#  define PIN_POWER     5
#  define PIN_FOCUS     6
#  define PIN_SHUTTER   7
#elif defined(BOARD_MEGA) || defined(BOARD_MEGA2560)
#  define PIN_DETECT    3
#  define PIN_POWER	26
#  define PIN_FOCUS	28
#  define PIN_SHUTTER   30
#else
#  error "Don't know which pins to use on this board!"
#endif

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
        if (cam_check_power())
            goto power_off;
        warn(WLOG, "cam: taking a picture");
        cam_shot        = 1;
        cam_power();
        return TASK_DELAY(3000); //3 secs to allow lens to extend and power on

    case CAM_FOCUS:
        if (!cam_check_power())
            goto power_on;
        warn(WDEBUG, "CAM_FOCUS");
        digitalWrite(PIN_FOCUS, HIGH);
        return TASK_DELAY(2000); //2 secs to allow focus

    case CAM_SHUTTER:
        if (!cam_check_power())
            goto power_on;
        warn(WDEBUG, "CAM_SHUTTER");
        digitalWrite(PIN_SHUTTER, HIGH);
        return TASK_DELAY(200); //Brief hold of button XXX how long?

    case CAM_STORE:
        if (!cam_check_power())
            goto power_on;
        warn(WDEBUG, "CAM_STORE");
        digitalWrite(PIN_SHUTTER, LOW);
        digitalWrite(PIN_FOCUS, LOW);
        if(cam_shot++ < 5)
            cam_state   = CAM_FOCUS;
        return TASK_DELAY(3000); //3 secs to store image on card

    case CAM_POWEROFF:
        if (cam_check_power()) {
            warn(WDEBUG, "CAM_POWEROFF");
            cam_power();
            return TASK_DELAY(3000); // 3s to complete powerdown
        }
        /* fall through */

    case CAM_FINISH:
        warn(WDEBUG, "CAM_FINISH");
        if (cam_check_power())
            goto power_off;
        cam_state       = CAM_START;
        /* XXX ~5 mins until next shots? */
        return TASK_DELAY(5*60*1000L);

    default:
        warn(WERROR, "Camera task: bad state");
        if (cam_check_power())
            cam_power();
        cam_state       = CAM_START;
        return TASK_DELAY(3*60*1000L);
    }

  power_off:
    warn(WWARN, "Camera was powered on unexpectedly");
    cam_power();
  power_on:
    warn(WERROR, "Camera power is incorrect, trying to recover");
    cam_state = CAM_START;
    return TASK_DELAY(3000);
}

static byte
cam_check_power ()
{
    return digitalRead(PIN_DETECT);
}

static void 
cam_power (void)
{
    digitalWrite(PIN_POWER, HIGH);
    delay(200); //XXX how long a hold - experiment!
    digitalWrite(PIN_POWER, LOW);
}
