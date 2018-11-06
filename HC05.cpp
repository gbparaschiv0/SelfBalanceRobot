#include <Arduino.h>
#include <SoftwareSerial.h>

#define RX 2
#define TX 3
// Led pin define
#define LED 13

SoftwareSerial HC05(RX, TX);		// RX, TX
uint32_t loopTimer;
bool bOverTimeAlertLed = false;		// Semaphore in case the loop is taking longer to finish

bool bStart;
char value;
char buffLen;
unsigned short dataXY[2];		// x = 0, y = 1
unsigned short myX;

void setup(void) {
	pinMode(LED, OUTPUT);
	digitalWrite(LED, LOW);
	Serial.begin(115200);

	HC05.begin(9600);

	loopTimer = micros() + 4000;
}



void loop(void) {

	if (HC05.available()) {

		value = (char) HC05.read();

		if (value == 2) {
			bStart = true;
			buffLen = 0;
			dataXY[0] = 0;
			dataXY[1] = 0;
		} else if (value == 3) {
			bStart = false;
			if (buffLen <= 6) {
				myX = dataXY[0];
			}
			// Processing data
		} else if (bStart) {
			if (buffLen < 6) {
				dataXY[buffLen / 3] *= 10;
				dataXY[buffLen / 3] += value - 48;
			}

			buffLen++;
		}
	}

	Serial.println(myX);

	if (bOverTimeAlertLed)
		digitalWrite(LED, HIGH);
	bOverTimeAlertLed = true;

	while (loopTimer > micros())
		bOverTimeAlertLed = false;
	loopTimer += 4000;
}
