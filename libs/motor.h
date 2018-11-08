/*
 * motor.h
 *
 *  Created on: Nov 8, 2018
 *      Author: Maxfresh
 */

#ifndef LIBS_MOTOR_H_
#define LIBS_MOTOR_H_

// H-Bridge pins defines
#define LEFT 10
#define RIGHT 11

#define L_FW 9
#define L_BW 8
#define R_FW 6
#define R_BW 7


typedef enum
{
	bw,
	fw
}wheelDir;

extern void MotorInit(void);
extern void LeftMotorDir(wheelDir);
extern void RightMotorDir(wheelDir);

#endif /* LIBS_MOTOR_H_ */
