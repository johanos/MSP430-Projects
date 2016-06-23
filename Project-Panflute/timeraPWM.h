/*
 * timeraPWM.h
 *
 *  Created on: Apr 20, 2016
 *      Author: Herb
 */

#ifndef PROJECT450_B_TIMERAPWM_H_
#define PROJECT450_B_TIMERAPWM_H_

/**
	PWM config for timer A. By default, this uses ACLK as an input clock source.
	period - The period of the pulse
	duty_cycle - The duty_cycle of the pulse
	output_bits - Where to route timer A to.
*/
void init_pwm_timer0(unsigned int period, unsigned char duty_cycle, unsigned char output_bit);
void init_pwm_timer1(unsigned int period, unsigned char duty_cycle, unsigned char output_bit);
void change_tone_volume_output0(unsigned int half_period, unsigned char duty_cycle);
void change_tone_volume_output1(unsigned int half_period, unsigned char duty_cycle);

#endif /* PROJECT450_B_TIMERAPWM_H_ */
