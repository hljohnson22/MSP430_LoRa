/*
 * timer.h
 *
 *  Created on: Jun 9, 2021
 *      Author: dragy
 */
#ifndef TIMER_H_
#define TIMER_H_

#include <msp430.h>

void init_timer(void);

void sleep_pins(void);

void sleep_10_min(void);


#endif /* TIMER_H_ */
