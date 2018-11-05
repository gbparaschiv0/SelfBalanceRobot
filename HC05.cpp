#include <Arduino.h>
#include <SoftwareSerial.h>

#define RX 2
#define TX 3
// Led pin define
#define LED 13

SoftwareSerial HC05(RX, TX);		// RX, TX
uint32_t loopTimer;
bool bOverTimeAlertLed = false;		// Semaphore in case the loop is taking longer to finish
char readChar;

void setup(void) {
	pinMode(LED, OUTPUT);
	digitalWrite(LED, LOW);
	Serial.begin(115200);

	HC05.begin(9600);

	loopTimer = micros() + 4000;
}

void loop(void) {

	while (HC05.available()) {
		readChar = (char)HC05.read();
		Serial.print(readChar);
	}

	if (bOverTimeAlertLed)
		digitalWrite(LED, HIGH);
	bOverTimeAlertLed = true;

	while (loopTimer > micros())
		bOverTimeAlertLed = false;
	loopTimer += 4000;
}
