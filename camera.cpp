//First attempt at switching camera on with BJT

#include <Arduino.h>

#include "camera.h"
#include "task.h"
#include "warn.h"

static void   cam_setup	(void);
static void   cam_shoot	(void);
static void   cam_power	(void);

task cam_task = {
    .name   = "camera",
    .when   = 0,

    .setup  = cam_setup,
    .run    = cam_shoot,
    .reset  = 0,
};

static void cam_setup()
{
  pinMode(POWERPIN,OUTPUT);
  digitalWrite(POWERPIN,LOW);
  pinMode(FOCUSPIN,OUTPUT);
  digitalWrite(FOCUSPIN,LOW);
  pinMode(SHUTTER,OUTPUT);
  digitalWrite(SHUTTER,LOW);
}

static void cam_shoot()
{
  cam_power();
  
  digitalWrite(FOCUSPIN,HIGH);
  delay(3000);
  digitalWrite(SHUTTER,HIGH);
  delay(10000);
  digitalWrite(SHUTTER,LOW);
  digitalWrite(FOCUSPIN,LOW);
  delay(2000);
  
  cam_power();
}

static void cam_power()
{
  digitalWrite(POWERPIN,HIGH);
  delay(200);
  digitalWrite(POWERPIN,LOW);
}
