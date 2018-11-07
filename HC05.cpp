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
void ReadPhoneOutput(short *, short *, char *);
bool bOverTimeAlertLed = false;	// Semaphore in case the loop is taking longer to finish

short myX, myY;

char buttonCommand;

void setup(void)
{
	pinMode(LED, OUTPUT);
	digitalWrite(LED, LOW);
	Serial.begin(115200);

	HC05.begin(9600);

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

void ReadPhoneOutput(short * outputX, short * outputY, char * outCommand)
{
	static bool bStart;
	char value;
	static char buffLen;
	static unsigned short dataXY[2];		// x = 0, y = 1

	if (HC05.available())
	{

		value = (char) HC05.read();
		if (value == 2)
		{
			bStart = true;
			buffLen = 0;
			memset(dataXY, 0x00, sizeof(dataXY));
		}
		else if (value == 3)
		{
			bStart = false;
			if (buffLen == 6)
			{
				*outputX = (short) dataXY[0] - 200;
				*outputY = (short) dataXY[1] - 200;
			}
			else if (buffLen == 1 && NR_TO_CH(dataXY[0]) > 64)
			{
				*outCommand = NR_TO_CH(dataXY[0]);
			}
		}
		// Processing data
		else if (bStart)
		{
			if (buffLen < 6)
			{
				dataXY[buffLen / 3] *= 10;
				dataXY[buffLen / 3] += CH_TO_NR(value);
			}

			buffLen++;
		}
	}
}
