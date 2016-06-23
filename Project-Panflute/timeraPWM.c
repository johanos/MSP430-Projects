#include "timerapwm.h"
#include <msp430.h>

void init_pwm_timer0(unsigned int period, unsigned char duty_cycle, unsigned char output_bit)
{
	P1DIR |= output_bit; // Pin 1.2 output
	P1SEL |= output_bit; // Route Timer_A to P1.2

	TA0CTL = TASSEL_2 | MC_1;
	TA0CCR0 = period;
	TA0CCTL1 = OUTMOD_7; //Reset / Set (see page 364 of the user guide)
	TA0CCR1 = duty_cycle;
}

void init_pwm_timer1(unsigned int period, unsigned char duty_cycle, unsigned char output_bit)
{
	P2DIR    |=  output_bit;             // Set P2.4 to output-direction
    P2SEL    |=  output_bit;             // Set selection register 1 for timer-function


	TA1CTL   = TASSEL_2 | MC_1;
	TA1CCR0  = period;
	TA1CCTL1 = OUTMOD_7; //Reset / Set (see page 364 of the user guide)

	TA1CCR1  = duty_cycle; //DON'T USE THIS ONE ITS LIKE QUIET FOR SOME REASON
	TA1CCR2  = duty_cycle; //CHANGE THIS ONE IN CODE PLS SVP PORFAVOR
	TA1CCTL2 = OUTMOD_7;

}

void change_tone_volume_output0(unsigned int half_period, unsigned char duty_cycle)
{
	TA1CCR2 = duty_cycle ;		// Changes volume
	TA1CCR0 = half_period;		// Changes note
}

void change_tone_volume_output1(unsigned int half_period, unsigned char duty_cycle)
{
	TA0CCR1 = duty_cycle ;		// Changes volume
	TA0CCR0 = half_period;		// Changes note
}

