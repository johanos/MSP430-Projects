
asm(" .length 10000");
asm(" .width 132");
#include <msp430g2553.h>

// to use the strlen function in the function tx_start_string below
#include <string.h>

/*
 * initialize the USCI UART interface
 */

/*
 * UART Timing Parameters
 * The UART will be driven by the SMCLK at 8Mhz
 * The desired baudrate in this example is 9600
 */
#define SMCLKRATE 8000000
#define BAUDRATE 9600
/*
 * calculate the prescalar and "modulation":
 * prescaler = CLOCK/baudrate
 * BRDIV = integer part of prescalar
 * modulation = Round (8* fractional part of prescaler)
 *
 */
#define BRDIV16 ((16*SMCLKRATE)/BAUDRATE)
#define BRDIV (BRDIV16/16)
#define BRMOD ((BRDIV16-(16*BRDIV)+1)/16)
#define BRDIVHI (BRDIV/256)
#define BRDIVLO (BRDIV-BRDIVHI*256)

//Port 1 pins used for transmit and receive are P1.2 and P1.1
#define TXBIT 0x04
#define RXBIT 0x02

void init_USCI_UART(){
	UCA0CTL1 = UCSWRST;   // reset and hold UART state machine
	UCA0CTL1 |= UCSSEL_2; // select SMCLK as the clock source
	UCA0BR1= BRDIVHI;      // set baud parameters, prescaler hi
	UCA0BR0= BRDIVLO;	  // prescaler lo
	UCA0MCTL=2*BRMOD;     // modulation
	// setup the TX pin (connect the P1.2 pin to the USCI)
	P1SEL |= TXBIT;
	P1SEL2|= TXBIT;
	UCA0CTL1 &= ~UCSWRST; // allow the UART state machine to operate
}

/*
 * Transmit subsystem
 * The TX interrupt handler is invoked when the transmit buffer is available to receive
 * a character AND transmit interrupts are enabled.
 *
 * The basic handler below implements an engine which maintains a pointer to a character array
 * and a counter for how many characters to send.  When the handler is invoked, if there are characters
 * remaining to be sent, it sends one more, advances the pointer, and decrements the counter.
 * If the count is zero, then the handler disables the TX interrupt and returns.
 */

char *tx_next_character;  // pointer to the next character to transmit
int tx_count;             // remaining number of characters to transmit

int tx_buffer_count(){
	return tx_count;
}

// UART Transmit interrupt handler:

void interrupt tx_handler(){
	if (tx_count>0){ // are there characters left to transmit?
		--tx_count;  // decrement the count
		UCA0TXBUF = *tx_next_character++; // send the current character & advance the pointer
	} else {         // when no characters left
		IE2 &= ~UCA0TXIE; // disable the transmit interrupt
	}
}
ISR_VECTOR(tx_handler,".int06") // declare interrupt vector

/*
 * Interface to START a transmission.
 * This function takes a pointer to a character array and a count of characters to send
 * and initiates transmission using the TX interrupt handler.
 * NOTES:
 * (1) this function returns immediately and DOES NOT wait for the transmission to complete.
 * (2) a user can tell if the transmission is still going on by noticing if tx_count is 0
 * (3) this function returns 0 if successful and -1 if it fails (because it will not interrupt
 *     a transmission already in progress).
 */
int tx_start(char *buffer, int count){
	if(tx_count==0){          // check that a previous transmission is not still in progress
		tx_count=count;       // store parameters for the transmit interrupt
		tx_next_character=buffer;
		IE2  |= UCA0TXIE;     // enable the transmit interrupt (this will immediately generate the 1st interrupt)
		return 0;             // success (transmission started)
	} else {                  // busy error
		return -1;            // failure
	}
}


// Syntatic sugar to start transmitting a C string
int tx_start_string(char *s){
	return tx_start(s,strlen(s));
}

/*
 * For restricted use only -- start printing a string and then WAIT for completion!
 * this simply does a busy-wait.  Intended for initialization and setup!
 */
void u_print_string(char *s){
	while (tx_start_string(s)){};// busy wait for successful start!
	while (tx_count>0){}; // busy wait for completion
}

