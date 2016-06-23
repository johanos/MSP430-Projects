/* 4-1-2015
 SPI_SLAVE
 Receives a '<', '>', or '=' symbol from the master and sends back a guess
*/

#include "msp430g2553.h"
#include <math.h>

 /* declarations of functions defined later */
 void init_spi(void);
 void init_WDT(void);
// Global variables and parameters (all volatilel to maintain for debugger)

volatile unsigned long tx_count = 0;		// total number of transmissions
volatile unsigned char data_received = 0; 	// most recent byte received
volatile unsigned long rx_count = 0;			// total number received handler calls
char complete = 0;
volatile int guess = 127;
volatile int ub = 255;
volatile int lb = 0;
char guessMSByte;
char guessLSByte;
int guessManip;
int WDT_interval = 60;
char startCondition = 0;
char can_update = 1;

// Try for a fast send.  One transmission every 64 microseconds
// bitrate = 1 bit every 4 microseconds
#define ACTION_INTERVAL 1
#define BIT_RATE_DIVISOR 32

volatile unsigned int action_counter=ACTION_INTERVAL;

// ======== Receive interrupt Handler for UCB0 ==========

	//guesslower = guess & 0x00FF
	// guessupper = guess / 256

	// if(state == 1) TXBUF = guesslower
	// else TXBUF = guesshigher

	// toggle state

void interrupt spi_rx_handler(){
	UCB0TXBUF = guess;
	while (!(IFG2 & UCB0TXIFG));			// Wait until the slave has finished transmitting (send and receive happen at the same time)
	data_received = UCB0RXBUF;			// Write into data_received the value that the master clocked into the slave's RX buffer

	if (data_received == '<') {
		guess++;
		//lb = guess;
		//guess = (lb+ub)/2;
	}
	if (data_received == '>') {
		guess--;
		//ub = guess;
		//guess = (lb+ub)/2;
	}
	if (data_received == '=') {
		P1OUT |= 0x01;
	}
}
ISR_VECTOR(spi_rx_handler, ".int07")

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
										// slave
			   +UCMODE_0				// 3-pin SPI mode
			   +UCSYNC;					// sync mode (needed for SPI or I2C)
	UCB0BR0=BRLO;						// set divisor for bit rate
	UCB0BR1=BRHI;
	UCB0CTL1 &= ~UCSWRST;				// enable UCB0 (must do this before setting
										//              interrupt enable and flags)
	IFG2 &= ~UCB0RXIFG;					// clear UCB0 RX flag
	IE2 |= UCB0RXIE;					// enable UCB0 RX interrupt
	// Connect I/O pins to UCB0 SPI
	P1SEL |=SPI_CLK+SPI_SOMI+SPI_SIMO;
	P1SEL2|=SPI_CLK+SPI_SOMI+SPI_SIMO;
}



/*
 * The main program just initializes everything and leaves the action to
 * the interrupt handlers!
 */

void main(){
	WDTCTL = WDTPW + WDTHOLD;       // Stop watchdog timer
	BCSCTL1 = CALBC1_8MHZ;			// 1Mhz calibration for clock (set a slower clock for two way communication)
  	DCOCTL  = CALDCO_8MHZ;
  	P1DIR |= 0x01;
  	P1OUT &= !0x01;
  	init_spi();
  	UCB0TXBUF = guess;				// At the start of the first slave receive handler, guess NEEDS to be in the slave transmit buffer!
 	_bis_SR_register(GIE+LPM0_bits);
}
