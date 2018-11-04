#include <Arduino.h>
#include "libs/I2Cdev.h"
#include "libs/MPU6050.h"

#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

#define DEBUG_NO_MOTOR_SPIN 0

#define LED 13

#define TRUE 1
#define FALSE 0

#define LEFT 10
#define RIGHT 11

#define L_FW 9
#define L_BW 8
#define R_FW 6
#define R_BW 7

#define FW 1
#define BW 0

// MPU6050 Variables
int16_t ax, ay, az;
int16_t gx, gy, gz;
int32_t gcx, gcy, gcz;
float angleY, accVector;

// PID Variables
float pid_error_temp, pid_i_mem, pid_setpoint, pid_output, pid_last_d_error;
uint16_t pid_converted;

// PID values
float pid_p_gain = 12;
float pid_i_gain = 0.001;
float pid_d_gain = 3;

uint32_t loop_timer;
char overTimeAlertLed = 0;

char bMotorToggle = 0;

MPU6050 accelgyro;

void MotorInit(void);
void LeftMotorDir(char);
void RightMotorDir(char);

void setup(){
	pinMode(LED, OUTPUT);

    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif
	Serial.begin(115200);

	accelgyro.initialize();			// MPU6050 Init


	accelgyro.setDLPFMode(3);		// 44Hz Low pass filter
	accelgyro.setFullScaleGyroRange(1);

	// Setting MPU6050 offsets to 0
	accelgyro.setXGyroOffset(0);
    accelgyro.setYGyroOffset(0);
    accelgyro.setZGyroOffset(0);

	MotorInit();					// Init motors

	loop_timer = micros() + 4000;

	for(int i = 0; i < 2000; i++)
	{
		if(i % 15 == 0)digitalWrite(LED, !digitalRead(LED));

		gcy += accelgyro.getRotationY();

		while(loop_timer > micros());
		loop_timer += 4000;
	}
	gcy /= 2000;

	// Get acc start values
	accelgyro.getAcceleration(&ax, &ay, &az);

	// Calculate initial angle
	accVector = (float)sqrt(((uint32_t)ax * ax) + ((uint32_t)ay * ay) + ((uint32_t)az * az));
	angleY = (-1) * asin(ax / accVector) * RAD_TO_DEG;

	digitalWrite(LED, LOW);
	loop_timer = micros() + 4000;
}

void loop()
{
	gy = accelgyro.getRotationY() - gcy;			// Get gyro Y data and substract calibration value
	angleY += ((float)gy / 65.5)/250;				// Convert in degree




	///////////////////////////////////////////////////////////////////////////////////////////////////
	//  PID section


	pid_error_temp = (int16_t)(angleY);

	pid_i_mem += pid_i_gain * pid_error_temp;
	if(pid_i_mem > 150)pid_i_mem = 150;
	else if(pid_i_mem < -150)pid_i_mem = -150;

	//Calculate the PID output value
	pid_output = pid_p_gain * pid_error_temp + pid_i_mem + pid_d_gain * (pid_error_temp - pid_last_d_error);
	if(pid_output > 300)pid_output = 300;                                     //Limit the PI-controller to the maximum controller output
	else if(pid_output < -300)pid_output = -300;

	pid_last_d_error = pid_error_temp;                                        //Store the error for the next loop

	if(angleY < 3 && angleY > -3) 		//Create a dead-band to stop the motors when the robot is balanced
	{
		pid_converted = 0;
		pid_output = 0;
		pid_i_mem = 0;			// Need more testes if this needs to be reseted or not in Dead-band
		analogWrite(LEFT, 0);
		analogWrite(RIGHT, 0);
	}

	if(angleY > 25 || angleY < -25)
	{
		pid_output = 0;
		pid_converted = 0;
		pid_i_mem = 0;
		analogWrite(LEFT, 0);
		analogWrite(RIGHT, 0);
	}

	if(pid_output < 0)
	{
		(void)LeftMotorDir(BW);
		(void)RightMotorDir(BW);
		pid_output *= -1;
	}
	else
	{
		(void)LeftMotorDir(FW);
		(void)RightMotorDir(FW);
	}

	if(pid_output != 0)
	{
		pid_converted = map((uint16_t)(pid_output), 1, 300, 65, 255);
#if DEBUG_NO_MOTOR_SPIN != 1
		if(bMotorToggle)
		{
			analogWrite(LEFT, pid_converted);
			analogWrite(RIGHT, pid_converted);
		}
		else
		{
			analogWrite(RIGHT, pid_converted);
			analogWrite(LEFT, pid_converted);
		}
		bMotorToggle ^= 0x01;
#endif
	}

	Serial.print("An: "); Serial.print(angleY);
	Serial.print("\tPID_Res: "); Serial.print(pid_output);
	Serial.print("\tPID: "); Serial.println(pid_converted);
	//Serial.println(pid_converted);

	// Visual led alert in case 250Hz loop is lager
	if(overTimeAlertLed) digitalWrite(LED, HIGH);
	overTimeAlertLed = 1;

	while(loop_timer > micros()) overTimeAlertLed = 0;
	loop_timer += 4000;
}

void MotorInit(void)
{
	pinMode(LEFT, OUTPUT);
	pinMode(RIGHT, OUTPUT);

	pinMode(L_FW, OUTPUT);
	pinMode(L_BW, OUTPUT);
	pinMode(R_FW, OUTPUT);
	pinMode(R_BW, OUTPUT);
	// Set initial rotation direction
	(void)LeftMotorDir(FW);
	(void)RightMotorDir(FW);
}

void LeftMotorDir(char dir)
{
	if(dir == BW)
	{
		digitalWrite(L_BW, HIGH);
		digitalWrite(L_FW, LOW);
	}
	else if(dir == FW)
	{
		digitalWrite(L_FW, HIGH);
		digitalWrite(L_BW, LOW);
	}
}

void RightMotorDir(char dir)
{
	if(dir == BW)
	{
		digitalWrite(R_BW, HIGH);
		digitalWrite(R_FW, LOW);
	}
	else if(dir == FW)
	{
		digitalWrite(R_FW, HIGH);
		digitalWrite(R_BW, LOW);
	}
}
