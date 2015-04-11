//First attempt at switching camera on with BJT

#ifndef __RADIO_H
#define __RADIO_H

#define RADIO_PIN   11
#define ENABLE_PIN  8

#define RTTY_BAUD   50

#include "task.h"

extern task radio_task;

#endif
