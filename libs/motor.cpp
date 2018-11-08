/*
 * motor.c
 *
 *  Created on: Nov 8, 2018
 *      Author: Maxfresh
 */
#include "motor.h"
#include <Arduino.h>

void MotorInit(void)
{
	pinMode(LEFT, OUTPUT);
	pinMode(RIGHT, OUTPUT);

	pinMode(L_FW, OUTPUT);
	pinMode(L_BW, OUTPUT);
	pinMode(R_FW, OUTPUT);
	pinMode(R_BW, OUTPUT);
	// Set initial rotation direction
	(void) LeftMotorDir(fw);
	(void) RightMotorDir(fw);
}

void LeftMotorDir(wheelDir dir)
{
	if (dir == bw)
	{
		digitalWrite(L_BW, HIGH);
		digitalWrite(L_FW, LOW);
	}
	else if (dir == fw)
	{
		digitalWrite(L_FW, HIGH);
		digitalWrite(L_BW, LOW);
	}
}

void RightMotorDir(wheelDir dir)
{
	if (dir == bw)
	{
		digitalWrite(R_BW, HIGH);
		digitalWrite(R_FW, LOW);
	}
	else if (dir == fw)
	{
		digitalWrite(R_FW, HIGH);
		digitalWrite(R_BW, LOW);
	}
}
