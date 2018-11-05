#include <Arduino.h>
#include <SoftwareSerial.h>

#define RX 2
#define TX 3

SoftwareSerial HC05(RX, TX);		// RX, TX

void setup(void) {
	Serial.begin(115200);

	HC05.begin(9600);
}

void loop(void) {
	if (HC05.available()) {
		Serial.write(HC05.read());
	}
}
