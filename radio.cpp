//First attempt at switching camera on with BJT

#include <Arduino.h>

#include "radio.h"
#include "task.h"
#include "warn.h"

static void   radio_setup	(void);
static void   radio_tx  	(unsigned long now);

task radio_task = {
    .name   = "radio",
    .when   = 0,

    .setup  = radio_setup,
    .run    = radio_tx,
    .reset  = 0,
};

static void 
radio_setup (void)
{
    //?
}

static void 
radio_tx (unsigned long now)
{
    //?
}
