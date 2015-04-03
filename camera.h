//First attempt at switching camera on with BJT

#ifndef __CAMERA_H
#define __CAMERA_H

#define POWERPIN	7
#define FOCUSPIN	9
#define SHUTTER		12
#define TIMEGAP		3

#include "task.h"

extern task cam_task;

#endif
