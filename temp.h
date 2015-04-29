/* temp.h
 * Read the temperature sensors over the one-wire bus.
 */

#ifndef __TEMP_H
#define __TEMP_H

#include "task.h"

#define NTEMP       3

extern task     temp_task;

extern byte     temp_ntemp;
extern float    temp_last_reading[NTEMP];

#endif
