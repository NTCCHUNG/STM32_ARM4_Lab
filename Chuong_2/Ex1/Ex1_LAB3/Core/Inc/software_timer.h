/*
 * software_timer.h
 *
 *  Created on: Nov 29, 2024
 *      Author: ADMIN
 */

#ifndef INC_SOFTWARE_TIMER_H_
#define INC_SOFTWARE_TIMER_H_

#include "software_timer.h"

#include "tim.h"
#include "led_7seg.h"

extern uint16_t flag_timer2;

void timer_init();
void setTimer2(uint16_t duration);

#endif /* INC_SOFTWARE_TIMER_H_ */
