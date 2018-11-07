/*
 * main.cpp
 *
 *  Created on: Nov 7, 2018
 *      Author: Maxfresh
 */
#include <Arduino.h>
#include <SoftwareSerial.h>
#include "libs/HC05.h"

#define LED 13

bool bOverTimeAlertLed = false;	// Semaphore in case the loop is taking longer to finish
uint32_t loopTimer;
short myX, myY;
char buttonCommand;

void setup(void)
{
	pinMode(LED, OUTPUT);
	digitalWrite(LED, LOW);
	Serial.begin(115200);

	(void) HC05_Init();

	loopTimer = micros() + 4000;
}

void loop(void)
{

	(void) ReadPhoneOutput(&myX, &myY, &buttonCommand);

	Serial.print(myX);
	Serial.print("\t");
	Serial.println(myY);

	if (buttonCommand)
	{
		Serial.println(buttonCommand);
		buttonCommand = 0;
	}

	if (bOverTimeAlertLed)
		digitalWrite(LED, HIGH);
	bOverTimeAlertLed = true;

	while (loopTimer > micros())
		bOverTimeAlertLed = false;
	loopTimer += 4000;
}
