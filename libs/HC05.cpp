#include <Arduino.h>
#include <SoftwareSerial.h>
#include "HC05.h"

#define RX 2
#define TX 3
#define START_OF_STRING 2
#define END_OF_STRING 3

#define NR_TO_CH(x) (x + 48)		// Convert number to ASCII
#define CH_TO_NR(x) (x - 48)		// Convert ASCII to number

SoftwareSerial HC05(RX, TX);		// RX, TX

void HC05_Init(void)
{
	HC05.begin(9600);
}

void ReadPhoneOutput(short * outputX, short * outputY, char * outCommand)
{
	static bool bStartRec;		// Semaphore for recording data
	char valueRecive;
	static char dataRecivedLen;
	static unsigned short dataRecived[2];		// Buffer for data received from phone

	if (HC05.available())
	{
		valueRecive = (char) HC05.read();		// Get value from phone
		if (valueRecive == START_OF_STRING)
		{
			// Reset all values and start recording
			bStartRec = true;
			dataRecivedLen = 0;
			memset(dataRecived, 0x00, sizeof(dataRecived));		// Reset array
		}
		else if (valueRecive == END_OF_STRING)
		{
			// Stop recording and process data
			bStartRec = false;
			if (dataRecivedLen == 6)		// If coordinates, process it
			{
				*outputX = (short) dataRecived[0] - 200;
				*outputY = (short) dataRecived[1] - 200;
			}
			else if (dataRecivedLen == 1 && NR_TO_CH(dataRecived[0]) > 64)		// If button
			{
				*outCommand = NR_TO_CH(dataRecived[0]);
			}
		}
		else if (bStartRec)		// Record data
		{
			if (dataRecivedLen < 6)		// Don't record more then 6 char
			{
				dataRecived[dataRecivedLen / 3] *= 10;
				dataRecived[dataRecivedLen / 3] += CH_TO_NR(valueRecive);
			}

			dataRecivedLen++;
		}
	}
}
