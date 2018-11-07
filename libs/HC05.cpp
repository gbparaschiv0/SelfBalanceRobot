#include <Arduino.h>
#include <SoftwareSerial.h>
#include "HC05.h"

#define RX 2
#define TX 3

#define NR_TO_CH(x) (x + 48)		// Convert number to ASCII
#define CH_TO_NR(x) (x - 48)		// Convert ASCII to number

SoftwareSerial HC05(RX, TX);		// RX, TX

void HC05_Init(void)
{
	HC05.begin(9600);
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
