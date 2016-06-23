/* 4-1-2015
 SPI_bounce_master
 At regular WDT intervals, this sends data out the UCB SPI interface.
 The data received is logged by the RX interrupt handler.
 This example can be used to loop back (ie, connecting MOSI to MISO)
 A global counter keeps track of the numbers of TX and RX operations.

 Timing and clock.
 MCLK and SMCLK = 8 Mhz
 UCB0BRx interface divisor is a parameterized below.
 WDT divides SMCL by 512 (==> fastest rate gives 1 TX every 64 microseconds)
 Parameter ACTION_INTERVAL controls actual frequency of WDT interrupts that TX
 16 bit Parameter BIT_RATE_DIVISOR controls the SPI bitrate clock
*/

#include "msp430g2553.h"
#include "uart.h"
#include <stdlib.h>
#include <time.h>

 /* declarations of functions defined later */
 void init_spi(void);
 void init_wdt(void);

// Global variables and parameters (all volatilel to maintain for debugger)

volatile unsigned char data_received= 0; 	// most recent byte received
volatile unsigned char last_button=0;
//volatile unsigned int counter;
//volatile unsigned int start=0;
char send_symbol = '<';				// The symbol to send to the master
char initial_value;					// The random value to start with

// Try for a fast send.  One transmission every 64 microseconds
// bitrate = 1 bit every 4 microseconds
#define ACTION_INTERVAL 1
#define BIT_RATE_DIVISOR 32
#define BUTTON 0x08
#define RED 0x01

// ===== Watchdog Timer Interrupt Handler ====

volatile unsigned int action_counter=ACTION_INTERVAL;
int state = 0;

interrupt void WDT_interval_handler(){
	if(state==1){
		UCB0TXBUF=send_symbol;							// Write to the slave in order to receive from it
		while(!UCB0RXIFG);						// Wait while the receive flag is high
		data_received = UCB0RXBUF;				// Read data from the buffer now that it is finished clocking everything in
		if (data_received == initial_value) {
			send_symbol = '=';
			tx_start_string("=");
			state =0;
		}
		if (data_received > initial_value) {
			send_symbol = '>';
			tx_start_string(">");
			state =1;
		}
		if (data_received < initial_value) {
			send_symbol = '<';
			tx_start_string("<");
			state =1;
		}
	}
	unsigned char b;
	b= (P1IN & BUTTON);  // read the BUTTON bit
	if (last_button && (b==0)){ // has the button bit gone from high to low
		state = 1;
		srand(time(0));
	 	int rand_num = rand() % 255;	// Pick a number between 0 and 255 (for now, later something closer to 65000)
	 	initial_value = (char) rand_num;	// Cast the integer to a char (for now)
	 	P1OUT |= RED;
	}
  	last_button=b;    // remember button reading for next time.
}
ISR_VECTOR(WDT_interval_handler, ".int10")

void init_wdt(){
	// setup the watchdog timer as an interval timer
	// INTERRUPT NOT YET ENABLED!
  	WDTCTL = (WDTPW +		// (bits 15-8) password
     	                   	// bit 7=0 => watchdog timer on
       	                 	// bit 6=0 => NMI on rising edge (not used here)
                        	// bit 5=0 => RST/NMI pin does a reset (not used here)
           	 WDTTMSEL +     // (bit 4) select interval timer mode
  		     WDTCNTCL  		// (bit 3) clear watchdog timer counter
  		                	// bit 2=0 => SMCLK is the source
  		                	// bits 1-0 = 10=> source/512
 			 );
  	IE1 |= WDTIE; // enable WDT interrupt
 }

//Bit positions in P1 for SPI
#define SPI_CLK 0x20
#define SPI_SOMI 0x40
#define SPI_SIMO 0x80

// calculate the lo and hi bytes of the bit rate divisor
#define BRLO (BIT_RATE_DIVISOR &  0xFF)
#define BRHI (BIT_RATE_DIVISOR / 0x100)

void init_spi(){
	UCB0CTL1 = UCSSEL_2+UCSWRST;  		// Reset state machine; SMCLK source;
	UCB0CTL0 = UCCKPH					// Data capture on rising edge
			   							// read data while clock high
										// lsb first, 8 bit mode,
			   +UCMST					// master
			   +UCMODE_0				// 3-pin SPI mode
			   +UCSYNC;					// sync mode (needed for SPI or I2C)
	UCB0BR0=BRLO;						// set divisor for bit rate
	UCB0BR1=BRHI;
	UCB0CTL1 &= ~UCSWRST;				// enable UCB0 (must do this before setting
										//              interrupt enable and flags)
	IFG2 &= ~UCB0RXIFG;					// clear UCB0 RX flag
	//IE2 |= UCB0RXIE;					// enable UCB0 RX interrupt <- nope, not going to use it.
	// Connect I/O pins to UCB0 SPI
	P1SEL |=SPI_CLK+SPI_SOMI+SPI_SIMO;
	P1SEL2|=SPI_CLK+SPI_SOMI+SPI_SIMO;
}

void init_button(){
	P1DIR &= ~BUTTON;
	P1OUT |= BUTTON;
	P1REN |= BUTTON;
}

/*
 * The main program just initializes everything and leaves the action to
 * the interrupt handlers!
 */

void main(){
	WDTCTL = WDTPW + WDTHOLD;       // Stop watchdog timer
	BCSCTL1 = CALBC1_8MHZ;			// 1Mhz calibration for clock
  	DCOCTL  = CALDCO_8MHZ;

  	P1DIR |= RED;
  	P1OUT &= ~RED;

  	//counter = ACTION_INTERVAL;
  	init_USCI_UART();
  	init_button();
  	init_spi();
  	init_wdt();
  	tx_start_string("Hi Daddy ;)");
 	_bis_SR_register(GIE+LPM0_bits);
}
