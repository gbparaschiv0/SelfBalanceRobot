#include "SelfBalanceRobot.h"

// Debugging defines
#define DEBUG_NO_MOTOR_SPIN 0
#define PID_CALIBRATION 0

void setup(void)
{
	pinMode(LED, OUTPUT);

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
	Wire.begin();
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
	Fastwire::setup(400, true);
#endif
	Serial.begin(115200);

#if PID_CALIBRATION == 1
	myPID = {10, 0, 0};
	EEPROM.put(0, myPID);
#else
	EEPROM.get(0, myPID);
#endif

	(void) HC05_Init();		// initialize Bluetooth HC05
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

#if PID_CALIBRATION == 1
	if(xOutput > 50)
	{
		EEPROM.put(0, myPID);
		delay(2000);
	}

	(void) ProcessButton();
#endif


	////////////////////////////////////////////////////////////////////////////
	//  PID section

	pidErrorTemp = angleY - pidSetpoint + selfBalancePidSetpoint;

	pidIMen += myPID.i * pidErrorTemp;
	if (pidIMen > 150)
		pidIMen = 150;
	else if (pidIMen < -150)
		pidIMen = -150;

	// Calculate the PID output value
	pidOutput = myPID.p * pidErrorTemp + pidIMen
			+ myPID.d * (pidErrorTemp - pidLastDError);
	if (pidOutput > 300)
		pidOutput = 300;		// Limit the PI-controller to the maximum controller output
	else if (pidOutput < -300)
		pidOutput = -300;

	pidLastDError = pidErrorTemp;		// Store the error for the next loop

	if (pidOutput < (3 * myPID.p) && pidOutput > (-3 * myPID.p))		// Create a dead-band to stop the motors when the robot is balanced
	{
		pidConverted = 0;
		pidOutput = 0;
	}

	if (angleY > 25 || angleY < -25)
	{
		pidOutput = 0;
		pidConverted = 0;
		pidIMen = 0;
		analogWrite(LEFT, 0);
		analogWrite(RIGHT, 0);
	}

#if PID_CALIBRATION == 0
	if (yOutput > 30)
	{
		if (pidSetpoint > -2.5)
			pidSetpoint -= 0.05;
		if (pidOutput > maxTargetSpeed * -1)
			pidSetpoint -= 0.005;

	}
	else if (yOutput < -30)
	{
		if (pidSetpoint < 2.5)
			pidSetpoint += 0.05;
		if (pidOutput < maxTargetSpeed)
			pidSetpoint += 0.005;

	}
	else		// Need to test if we should brake instant or slowly
	{
		if (pidSetpoint > 0.5)
			pidSetpoint -= 0.05;
		else if (pidSetpoint < -0.5)
			pidSetpoint += 0.05;
		else
			pidSetpoint = 0;
	}

	if (pidSetpoint == 0)
	{
		if (pidOutput < 0)
			selfBalancePidSetpoint += 0.0015;
		if (pidOutput > 0)
			selfBalancePidSetpoint -= 0.0015;
	}
#endif

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
	else
	{
		analogWrite(LEFT, 0);
		analogWrite(RIGHT, 0);
	}

/*		Serial.print("An: ");
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
	if (buttonCommand != NULL)
	{
		switch (buttonCommand)
		{
		case P_ADD:
		case P_ADD + 1:
			myPID.p += 1;
			break;

		case P_SUB:
		case P_SUB + 1:
			myPID.p -= 1;
			break;

		case I_ADD:
		case I_ADD + 1:
			myPID.i += (float) 1 / I_DEVISION;
			break;

		case I_SUB:
		case I_SUB + 1:
			myPID.i -= (float) 1 / I_DEVISION;
			break;

		case D_ADD:
		case D_ADD + 1:
			myPID.d += 1;
			break;

		case D_SUB:
		case D_SUB + 1:
			myPID.d -= 1;
			break;
		}
	}

	//EEPROM.put(0, myPID);
	// Reset button
	buttonCommand = NULL;
}
