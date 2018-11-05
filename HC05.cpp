#include <Arduino.h>
#include <SoftwareSerial.h>

#define RX 2
#define TX 3

SoftwareSerial HC05(RX, TX);		// RX, TX

void setup()
{
  pinMode(12, OUTPUT);  // this pin will pull the HC-05 pin 34 (key pin) HIGH to switch module to AT mode
  digitalWrite(12, HIGH);
  Serial.begin(9600);
  Serial.println("Enter AT commands:");
  HC05.begin(38400);  // HC-05 default speed in AT command more
}

void loop()
{

  // Keep reading from HC-05 and send to Arduino Serial Monitor
  if (HC05.available())
    Serial.write(HC05.read());

  // Keep reading from Arduino Serial Monitor and send to HC-05
  if (Serial.available())
	  HC05.write(Serial.read());
}
