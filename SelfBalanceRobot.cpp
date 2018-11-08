#include "SelfBalanceRobot.h"

// Debugging defines
#define DEBUG_NO_MOTOR_SPIN 0

// PID values
float pidPGain = 12;
float pidIGain = 0.001;
float pidDGain = 3;

void setup(void)
{
	pinMode(LED, OUTPUT);

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
	Wire.begin();
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
	Fastwire::setup(400, true);
#endif
	Serial.begin(115200);

	(void) HC05_Init();		// initialize bluetooth HC05
	(void) MotorInit();		// initialize motors

	////////////////////////////////////////////////////////////////////////////
	//  MPU6050 settings section
	accelgyro.initialize();		// MPU6050 initialize

	accelgyro.setDLPFMode(3);		// 44Hz Low pass filter
	accelgyro.setFullScaleGyroRange(1);		// Set gyroscope to -/+ 500 degree/second

	// Setting MPU6050 offsets to 0
	accelgyro.setXGyroOffset(0);
	accelgyro.setYGyroOffset(0);
	accelgyro.setZGyroOffset(0);

	////////////////////////////////////////////////////////////////////////////

	loopTimer = micros() + 4000;

	// Get gyroscope calibration value
	for (int i = 0; i < 2000; i++)
	{
		if (i % 15 == 0)
			digitalWrite(LED, !digitalRead(LED));
		gcy += accelgyro.getRotationY();
		while (loopTimer > micros())
			;
		loopTimer += 4000;
	}
	gcy /= 2000;

	// Get accelerometer start values
	accelgyro.getAcceleration(&ax, &ay, &az);

	// Calculate initial angle
	accVector = (float) sqrt(
			((uint32_t) ax * ax) + ((uint32_t) ay * ay) + ((uint32_t) az * az));
	angleY = (-1) * asin(ax / accVector) * RAD_TO_DEG;

	digitalWrite(LED, OFF);
	loopTimer = micros() + 4000;
}

void loop(void)
{
	gy = accelgyro.getRotationY() - gcy;// Get gyroscope Y data and subtract calibration value
	angleY += ((float) gy / 65.5) / 250;		// Convert in degree

	// Read data from phone
	(void) ReadPhoneOutput(&xOutput, &yOutput, &buttonCommand);


	////////////////////////////////////////////////////////////////////////////
	//  PID section

	pidErrorTemp = (int16_t) (angleY);

	pidIMen += pidIGain * pidErrorTemp;
	if (pidIMen > 150)
		pidIMen = 150;
	else if (pidIMen < -150)
		pidIMen = -150;

	// Calculate the PID output value
	pidOutput = pidPGain * pidErrorTemp + pidIMen
			+ pidDGain * (pidErrorTemp - pidLastDError);
	if (pidOutput > 300)
		pidOutput = 300;		// Limit the PI-controller to the maximum controller output
	else if (pidOutput < -300)
		pidOutput = -300;

	pidLastDError = pidErrorTemp;		// Store the error for the next loop

	if (angleY < 3 && angleY > -3)		// Create a dead-band to stop the motors when the robot is balanced
	{
		pidConverted = 0;
		pidOutput = 0;
		pidIMen = 0;		// Need more tests if this needs to be reseted or not in Dead-band
		analogWrite(LEFT, 0);
		analogWrite(RIGHT, 0);
	}

	if (angleY > 25 || angleY < -25)
	{
		pidOutput = 0;
		pidConverted = 0;
		pidIMen = 0;
		analogWrite(LEFT, 0);
		analogWrite(RIGHT, 0);
	}

	if (pidOutput < 0)
	{
		(void) LeftMotorDir(bw);
		(void) RightMotorDir(bw);
		pidOutput *= -1;
	}
	else
	{
		(void) LeftMotorDir(fw);
		(void) RightMotorDir(fw);
	}

	if (pidOutput != 0)
	{
		pidConverted = map((uint16_t) (pidOutput), 1, 300, 65, 255);
#if DEBUG_NO_MOTOR_SPIN != 1		
		if (motorToggle)
		{
			analogWrite(LEFT, pidConverted);
			analogWrite(RIGHT, pidConverted);
		}
		else
		{
			analogWrite(RIGHT, pidConverted);
			analogWrite(LEFT, pidConverted);
		}
		motorToggle ^= 0x01;
#endif
	}

	/*	Serial.print("An: ");
	 Serial.print(angleY);
	 Serial.print("\tPID_Res: ");
	 Serial.print(pidOutput);
	 Serial.print("\tPID: ");
	 Serial.println(pidConverted);*/

	// Visual led alert in case 250Hz loop is lager
	if (bOverTimeAlertLed)
		digitalWrite(LED, ON);
	bOverTimeAlertLed = 1;

	while (loopTimer > micros())
		bOverTimeAlertLed = 0;
	loopTimer += 4000;
}

void ProcessButton(void)
{
	if(buttonCommand != NULL)
	{
		//Process button
	}

	// Reset button
	buttonCommand = NULL;
}
