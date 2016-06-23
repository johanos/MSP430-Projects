#include <msp430g2553.h>
#include "timeraPWM.h"
#include "notes.h"

#define PWM_PERIOD 466
#define PWM_PERIOD_STEPS 10
#define PWM_PERIOD_STEP (PWM_PERIOD / PWM_PERIOD_STEPS)
#define PLAY_THRESHOLD 400

unsigned int ADC_Results[5]; //for holding the conversion results
unsigned int P10, P11, P13, P14;
unsigned int max1, max2 = 0;

struct adc_unit{
	int index;
	int magnitude;
} tempFirstHighest, tempSecondHighest, Max1, Max2;


void setup_adc()
{
	ADC10CTL1 = INCH_4 + CONSEQ_1+ ADC10SSEL_0;            					// A4/A3/A2/A1/A0, once multi channel
	ADC10CTL0 = REF2_5V + ADC10SHT_2 + MSC + REFON + ADC10ON + ADC10IE; 	//2.5 reference voltage, enable ADCISR
	ADC10AE0 = BIT0 + BIT1 + BIT3 + BIT4 /*0x1F*/;                         	// P1.0, 1, 3, 4 ADC option select
	ADC10DTC1 = 0x5;                         								// 5 conversions
}
int main(void)
{
	BCSCTL1 = CALBC1_1MHZ;
	DCOCTL  = CALDCO_1MHZ;
	Max1.index = -1;
	Max1.magnitude = 0;
	Max2.index = -1;
	Max2.magnitude = 0;


	// setup the watchdog timer as an interval timer
	WDTCTL =(WDTPW +	// (bits 15-8) password
	                    // bit 7=0 => watchdog timer on
	                    // bit 6=0 => NMI on rising edge (not used here)
	                    // bit 5=0 => RST/NMI pin does a reset (not used here)
	       WDTTMSEL +   // (bit 4) select interval timer mode
	       WDTCNTCL + 	// (bit 3) clear watchdog timer counter
	  		            // bit 2=0 => SMCLK is the source
	  		   2        // bits 1-0 = 10 => source/512 .
	);

	IE1 |= WDTIE;		// enable the WDT interrupt (in the system interrupt register IE1)

	 // in the CPU status register, set bits:
	 //    GIE == CPU general interrupt enable
	 //    LPM0_bits == bit to set for CPUOFF (which is low power mode 0)

	setup_adc();

	init_pwm_timer0(PWM_PERIOD, 0, BIT2); //Pin 2, port of port A. Initially off
	init_pwm_timer1((PWM_PERIOD*1.5), 0, BIT4);

	_bis_SR_register(GIE/*+LPM0_bits*/);  // after execution of this instruction, the CPU is off!
}


// ===== Watchdog Timer Interrupt Handler =====
// This event handler is called to handle the watchdog timer interrupt,
//    which is occurring regularly at intervals of 32K/8MHz ~= 4ms.

interrupt void WDT_interval_handler()
{
	// Take ADC Conversions into results array
	ADC10CTL0 &= ~ENC;
	while (ADC10CTL1 & BUSY);               // Wait if ADC10 core is active
	ADC10SA = (int)ADC_Results;             // Data buffer start
	ADC10CTL0 |= ENC + ADC10SC;             // Sampling and conversion ready
	P10 = ADC_Results[4];
	P11 = ADC_Results[3];
	P13 = ADC_Results[1];
	P14 = ADC_Results[0];




	tempFirstHighest.index      = -1;
	tempSecondHighest.index	    = -1;
	tempFirstHighest.magnitude  =  0;
	tempSecondHighest.magnitude =  0;


	// Use results of ADC polling to find highest values
	int i;
	for(i = 0; i < 5; i++)
	{
		if(i==2) continue;
		else
		{
			if(ADC_Results[i] > tempFirstHighest.magnitude)
			{
				tempSecondHighest.magnitude = tempFirstHighest.magnitude;
				tempSecondHighest.index     = tempFirstHighest.index	;

				tempFirstHighest.magnitude  = ADC_Results[i]			;
				tempFirstHighest.index      = i							;

				continue;

			}
			else if(ADC_Results[i] > tempSecondHighest.magnitude)
			{
				tempSecondHighest.magnitude = ADC_Results[i];
				tempSecondHighest.index 	= i;
			}
		}
	}

	// Assign Max Values
	Max1.index = tempFirstHighest.index;
	Max1.magnitude = tempFirstHighest.magnitude;
	Max2.index = tempSecondHighest.index;
	Max2.magnitude = tempSecondHighest.magnitude;

	// Play the highest threshold note from TimerA1
	if(Max1.magnitude > PLAY_THRESHOLD)
	{
		switch(Max1.index)
		{
			case 0:
			{
				change_tone_volume_output0(_F4, 50);
				break;
			}
			case 1:
			{
				change_tone_volume_output0(_E4, 50);
				break;
			}
			case 2:
			{
				change_tone_volume_output0(0, 0);
				break;
			}
			case 3:
			{
				change_tone_volume_output0(_D4, 50);
				break;
			}
			case 4:
			{
				change_tone_volume_output0(_C4, 50);
				break;
			}
			default:
			{
				change_tone_volume_output0(0, 0);
			}
		}
	}
	else
	{
		change_tone_volume_output0(0, 0);
	}


		// Play the second highest note from TimerA0
		if(Max2.magnitude > PLAY_THRESHOLD)
		{
			switch(Max2.index)
			{
				case 0:
				{
					change_tone_volume_output1(_F4, 50);
					break;
				}
				case 1:
				{
					change_tone_volume_output1(_E4, 50);
					break;
				}
				case 2:
				{
					change_tone_volume_output1(0, 0);
					break;
				}
				case 3:
				{
					change_tone_volume_output1(_D4, 50);
					break;
				}
				case 4:
				{
					change_tone_volume_output1(_C4, 50);
					break;
				}
				default:
				{
					change_tone_volume_output1(0, 0);
					break;
				}
			}
		}
		else
		{
			change_tone_volume_output1(0, 0);
		}
}

// DECLARE WDT_interval_handler as handler for interrupt 10
ISR_VECTOR(WDT_interval_handler, ".int10")

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
    //code from reading from res and sending it to AP
  //__bic_SR_register_on_exit(CPUOFF);        // Clear CPUOFF bit from 0(SR)
}

