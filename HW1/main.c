#include <msp430g2553.h>
/*
 * • == 1 | - == 3 | " " == 0
 *
 * H  - | • • • • |  4
 * <space>			 5
 * E  - | • |        6
 * <space>			 7
 * L  - | •  - • • | 11
 * <space>			 12
 * L  - | •  - • • | 16
 * <space>			 17
 * O  - | - - - |    20

<SPACE * 7>   		 23

 * J  - | • - - - |  27
 * <space>			 28
 * O  - | - - - |    31
 * <space>			 32
 * H  - | • • • • |  36
 * <space>			 37
 * A  - | • -  |     39
 * <space>			 40
 * N  - | - • |      42

*/

#define BUTTON BIT3
#define RED BIT0
#define GREEN BIT6

volatile const int MESSAGE_LENGTH = 45;
volatile unsigned char last_button;
//dot is 1 baseU, dash 3 baseU, space is 7 base U
const unsigned int u = 30;
const unsigned int u3 = 90;
const unsigned int u7 = 210;
					         // 	    |    H    | |  +  | | E | |  +  | |    L	   |  | + | |   L	|  |+|   |    O    |    <space>   |    J      | | + |  |    O   | | + | |   H   | | + | |  A  | | + | | N |

volatile unsigned int message[60] = {

    u, /*on for a unit	//DOT     */
    u, /*off for a unit 	      */

    u, /*on for a unit 	//DOT     */
    u, /*off for a unit		      */

	u, /* on for a unit 	      */
    u, /* off for a unit	//DOT */

	u, /* on for a unit	//DOT	  */
	u3, /* off for 3 units  letter ended  H	 */


	u,  /* on for 1 unit		 //DOT */
	u3, /* off for 3 units  letter ended E*/

	u, /* on 			//DOT */
	u, /* off */
	u3,/* on for 3  	//DASH */
	u, /* off for 1					*/
	u, /* on 1		//DOT		*/
	u, /* off	 				*/
	u, /* on 1 		//DOT		*/
	u3, /* OFF FOR 3  letter ended L */

	u, /* on 			//DOT	*/
	u, /* off 	*/
	u3,/* on for 3  	   //DASH */
	u, /*  off for 1		*/
	u, /* on 1		//DOT	*/
	u, /* off   	*/
	u, /* on 1 		//DOT	*/
	u3, /* OFF FOR 3 letter ended L */

	u3, /* on for 3 		//dash */
	u, 	/* off  for 1	*/
	u3, /* on for 3 		//dash */
	u, 	/* off  for 1	*/
	u3, /* on for 3 		//dash */
	u3, 	/* off  for 3 letter ended O AND word ended*/

	u, /* on 			//DOT	*/
	u, /* off 	*/
	u3, /* on for 3 		//dash */
	u, 	/* off  for 1	*/
	u3, /* on for 3 		//dash */
	u3, /* off  for 3  letter ended J	*/

	u3, /* on for 3 		//dash */
	u, 	/* off  for 1	*/
	u3, /* on for 3 		//dash */
	u, 	/* off  for 1	*/
	u3, /* on for 3 		//dash */
	u3, 	/* off  for 3 letter ended O */

	u, /*on for a unit	//DOT     */
	u, /*off for a unit 	      */

	u, /*on for a unit 	//DOT     */
	u, /*off for a unit		      */

	u, /* on for a unit 	      */
	u, /* off for a unit	//DOT */

	u, /* on for a unit	//DOT	  */
	u3, /* off for 3 units  letter ended  H	 */

	u, /*on for a unit 	//DOT     */
	u, /*off for a unit		      */
	u3, /*on for 3 units //DASH	  */
	u3, /* off for 3 units , letter ended A */

	u3, /* on for 3 units //DASH  */
	u, 	/*off for a unit */
	u,  /* on for a unit //DOT */
	u7 /*off for 7 letter ended N  MESSAGE ENDED*/
	};
volatile unsigned int counter;
volatile unsigned char last_button = 0;
volatile unsigned char LEDSelect = 0;
volatile unsigned char switchLight = 0; // is set to 1 if the user requests to change LEDs
volatile unsigned int indexWord = 0; // index of the message array
unsigned char mask[] = {RED, GREEN};




int main(void) {
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    // setup the watchdog timer as an interval timer
   	  WDTCTL =(WDTPW + // (bits 15-8) password
   	                   // bit 7=0 => watchdog timer on
   	                   // bit 6=0 => NMI on rising edge (not used here)
   	                   // bit 5=0 => RST/NMI pin does a reset (not used here)
   	           WDTTMSEL + // (bit 4) select interval timer mode
   	           WDTCNTCL +  // (bit 3) clear watchdog timer counter
   	  		          0 // bit 2=0 => SMCLK is the source
   	  		          +1 // bits 1-0 = 01 => source/8K
   	  		   );

    IE1 |= WDTIE; //Enables Watchdog timer

    P1DIR &= ~BUTTON; //sets the button in the input direction
    P1OUT |= BUTTON;
    P1REN |= BUTTON; //turns on the resistor of the button

    counter = message[indexWord]; //reading the letter from the message array

    _bis_SR_register(GIE+LPM0_bits);  // enable interrupts and also turn the CPU off!

	return 0;
}


// Watchdog Timer interrupt handler
// occurs at regular intervals of about 8K/1.1MHz ~= 7.4ms
interrupt void WDT_interval_handler(){

	unsigned char b;
	b = (P1IN & BUTTON); //reading the button bit

	if((last_button) && (b==0)){ //if it goes from high to low

		switchLight = 1; //variable that is set to 1 to perform the switching at the end of the message
		P1DIR |= mask[LEDSelect^1]; //sets the chosen LED in the output direction by XORing it
		P1OUT |= mask[LEDSelect^1]; //turns the new LED on to switch

	}

	P1DIR |= mask[LEDSelect]; // sets selected Light to output direction.

	if(--counter == 0) {
		P1OUT ^= mask[LEDSelect]; //toggles the selected LED
		counter = message[++indexWord]; //moves to the second character inside the string message by incrementing the index
	}

	if(indexWord == 40) { //if we reach the end of the message

		indexWord = 0; //make is start from the begining

		if(switchLight == 1) { //if the user requested to change LED

			P1OUT &= ~ mask[LEDSelect]; //turns the previous selected LED off
			LEDSelect ^= 1; // //toggles the LED select (if it is 0, we use RED, if it is 1 we use GREEN)
			switchLight = 0;
		}
	}

	last_button = b;
}

ISR_VECTOR(WDT_interval_handler, ".int10")





