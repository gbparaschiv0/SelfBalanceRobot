/*
 * HC05.h
 *
 *  Created on: Nov 7, 2018
 *      Author: Maxfresh
 */

#ifndef HC05_H_
#define HC05_H_

extern void ReadPhoneOutput(short * xOutput, short * yOutput,
		char * buttonCommand);
extern void HC05_Init(void);

#endif /* HC05_H_ */
