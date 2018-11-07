#include <Arduino.h>
#include <SoftwareSerial.h>

#define RX 2
#define TX 3
// Led pin define
#define LED 13

#define NR_TO_CH(x) (x + 48)		// Convert number to ASCII
#define CH_TO_NR(x) (x - 48)		// Convert ASCII to nr

SoftwareSerial HC05(RX, TX);		// RX, TX
uint32_t loopTimer;
bool bOverTimeAlertLed = false;	// Semaphore in case the loop is taking longer to finish

bool bStart;
char value;
char buffLen;
unsigned short dataXY[2];		// x = 0, y = 1
unsigned short myX;

char buttonCommand;
char temp[20];
char lastReadCounter;

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
		if (lastReadCounter > 19) {
			lastReadCounter = 0;
			memset(temp, 0x00, sizeof(temp));
		}

		temp[lastReadCounter] = value;
		lastReadCounter++;
		if (value == 2) {
			bStart = true;
			buffLen = 0;
			dataXY[0] = 0;
			dataXY[1] = 0;
		} else if (value == 3) {
			bStart = false;
			if (buffLen == 6) {
				myX = dataXY[0];
				if (myX < 100 || myX > 300) {
					Serial.println("Wrong value received:");
					for (int i = 0; i < 20; i++) {
						Serial.print((short) temp[i]);
						Serial.print("\t");
					}
					Serial.println();
					Serial.println("Converted:");
					for (int i = 0; i < 20; i++) {
						Serial.print((short) (temp[i] - 48));
						Serial.print("\t");
					}
					Serial.println();
					Serial.print("BuffLen: ");
					Serial.print((short) buffLen);
					while (1)
						;
				}
			}
			else if(buffLen == 1 && NR_TO_CH(dataXY[0]) > 64){
				buttonCommand = NR_TO_CH(dataXY[0]);
			}
		}
		// Processing data
		else if (bStart) {
			if (buffLen < 6) {
				dataXY[buffLen / 3] *= 10;
				dataXY[buffLen / 3] += CH_TO_NR(value);
			}

			buffLen++;
		}
	}
	//Serial.println(myX);
	if(buttonCommand){
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
