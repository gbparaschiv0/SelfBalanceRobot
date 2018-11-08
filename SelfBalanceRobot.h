/*
 * SelfBalanceRobot.h
 *
 *  Created on: Nov 8, 2018
 *      Author: Maxfresh
 */
#ifndef SELFBALANCEROBOT_H_
#define SELFBALANCEROBOT_H_

#include <Arduino.h>
#include <EEPROM.h>
#include "libs/I2Cdev.h"
#include "libs/MPU6050.h"
#include "libs/HC05.h"
#include "libs/motor.h"

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif

// Led pin define
#define LED 13

// Used for PID adjustment
#define P_ADD 65
#define P_SUB 67
#define I_ADD 69
#define I_SUB 71
#define D_ADD 73
#define D_SUB 75
#define I_DEVISION 1000

// User friendly defines
#define ON 1
#define OFF 0
#define NULL 0

typedef struct
{
	float p;
	float i;
	float d;
}sPID;

//Output values from phone
short xOutput, yOutput;
char buttonCommand;

// MPU6050 Variables
int16_t ax, ay, az;
int16_t gx, gy, gz;
int32_t gcx, gcy, gcz;
float angleY, accVector;

// PID Variables
float pidErrorTemp, pidIMen, pidOutput, pidLastDError;
uint16_t pidConverted;

uint32_t loopTimer;	// Store the exact value of microseconds every loop needs to have

bool bOverTimeAlertLed = false;	// Semaphore in case the loop is taking longer to finish
char motorToggle = 0;	// variable used to toggle the motors from left to right

MPU6050 accelgyro;

// PID structure
sPID myPID;

void ProcessButton(void);
#endif /* SELFBALANCEROBOT_H_ */
