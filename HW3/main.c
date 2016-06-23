#include <msp430g2553.h>

//function prototypes
void init_timerA(void);
void init_WDT(void);                        // setup WDT
void init_P1(void);                         // setup the I/O for Port 1
void toggle_pause(void);                    // toggle between paused and not-paused
void restart_song(void);                    // reset song from beginning, resets tempo
void increase_tempo(void);                  // increase the song tempo
void decrease_tempo(void);                  // decrease the song tempo
void select_song1(void);                    // select song 1, restart song from beginning
void select_song2(void);                    // select song 2, restart song from beginning
//end prototyps


/***************** global variables ****************************/
#define DEFAULT_TEMPO   2.25
#define FLASH_DURATION  29

unsigned char system_state = 2;                  // system state, 0 = playing, 1 = paused, 2 = start of music, 3 = end
unsigned int curr_song_len = SONG1_LENGTH;  	 // current song length, default to song1
const unsigned char *curr_song = song1;     	 // defaults value is pointing to "Joy to the World"
unsigned int WDT_duration = 0;         			 // counts the number of WDT cycles for the current note duration
unsigned int score_counter = 0;                  // indexes into the score for each note
unsigned char WDT_flash_duration = 0;            // counts the number of WDT cycles per LED flash
float tempo = DEFAULT_TEMPO;
unsigned char isbreak = 0;



/***********define the Port 1 bits*************/
#define LED             BIT0
#define SPEAKER         BIT1    // TA0 is on this pin, from the CCR0 register (output of capture compare register)
#define RESET_BUTTON    BIT2
#define PLAY_BUTTON     BIT3
#define SLOW_BUTTON     BIT4
#define FAST_BUTTON     BIT5
#define SONG1_BUTTON    BIT6
#define SONG2_BUTTON    BIT7

// frequencies from C4 to C6
/********define the note frequencies by half-period*************/
#define NUMBER_OF_NOTES   26

#define freqR      1 // Rest freq

#define freqC4     1911
#define freqC4s    1803
#define freqD4     1703
#define freqE4b    1607
#define freqE4     1517
#define freqF4     1432
#define freqF4s    1351
#define freqG4     1276
#define freqG4s    1204
#define freqA4     1136
#define freqB4b    1073
#define freqB4     1012
#define freqC5     956
#define freqC5s    902
#define freqD5     851
#define freqE5b    803
#define freqE5     758
#define freqF5     716
#define freqF5s    676
#define freqG5     638
#define freqG5s    602
#define freqA5     568
#define freqB5b    536
#define freqB5     506
#define freqC6     478
const unsigned int notes[NUMBER_OF_NOTES] = {
        freqR, // rest
		freqC4,freqC4s,freqD4,freqE4b,freqE4,freqF4,freqF4s,freqG4,freqG4s,freqA4,freqB4b,freqB4, // 5th octave
		freqC5,freqC5s,freqD5,freqE5b,freqE5,freqF5,freqF5s,freqG5,freqG5s,freqA5,freqB5b,freqB5, // 6th octave
		freqC6
};

/***** values for defining the frequency from the array above *****/
#define R       0x00
#define C4      0x01
#define C4s     0x02
#define D4      0x03
#define E4b     0x04
#define E4      0x05
#define F4      0x06
#define F4s     0x07
#define G4      0x08
#define G4s     0x09
#define A4      0x0A
#define B4b     0x0B
#define B4      0x0C
#define C5      0x0D
#define C5s     0x0E
#define D5      0x0F
#define E5b     0x10
#define E5      0x11
#define F5      0x12
#define F5s     0x13
#define G5      0x14
#define G5s     0x15
#define A5      0x16
#define B5b     0x17
#define B5      0x18
#define C6      0x19

//SCORE IS STORED AS A BYTE | S7 | S6 | S5 || S4 | S3 | S2 | S1 | S0 |
//							[   DURATION   ][ value to index array at]
#define NOTE_MASK   0x1F    // bit mask for isolating the note freqs from the durations in the score // anding the score with this
							// will set the duration bits to 0 and then only use the last 5 bits to figure out the index of the notes array to get


//note durations
/*********************************/
#define NUMBER__OF__DURATIONS   8


//These are how many watch dog timer interupts to hold a note for
const unsigned char durations[NUMBER__OF__DURATIONS] = {
        1,  // quick break between adjacent notes of the same frequency
        8,  // sixteenth note duration
        16, // eigth note
        24, // dotted eigth note
        32, // quarter note
        48, // dotted quarter note
        64, // half note
        96  // dotted half note
};

// bit mask for defining the score, these are the first three bits of the 8 bit score value.
// [000 00000]

#define BREAK            0x00    // break between adjacent notes
#define SIXTEENTH        0x20
#define EIGTH            0x40
#define DOTTED_EIGTH     0x60    // dotted eigth
#define QUARTER          0x80
#define DOTTED_QUARTER   0xA0    // dotted quarter
#define HALF             0xC0
#define DOTTED_HALF      0xE0    // dotted half



#define SONG1_LENGTH    72
// JOY TO THE WORLD BEGINS
//----------------------------------
unsigned const char song1[SONG1_LENGTH] = {
    (QUARTER + C5), (DOTTED_EIGTH + B4), (SIXTEENTH + A4), (DOTTED_QUARTER + G4),   // measure 2
    (EIGTH + F4), (QUARTER + E4), (QUARTER + D4), (DOTTED_QUARTER + C4), (EIGTH + G4), (DOTTED_QUARTER + A4),
    (BREAK + R), (EIGTH + A4), (DOTTED_QUARTER + B4), (BREAK + R), (EIGTH + B4), (DOTTED_QUARTER + C5),
    (BREAK + R), (EIGTH + C5), (BREAK + R), (EIGTH + C5), (EIGTH + B4), (EIGTH + A4),
    (EIGTH + G4), (BREAK + R), (DOTTED_EIGTH + G4), (SIXTEENTH + F4), (EIGTH + E4),
    (EIGTH + C5), (BREAK + R), (EIGTH + C5), (EIGTH + B4), (EIGTH + A4), (EIGTH + G4),
    (BREAK + R), (DOTTED_EIGTH + G4),  (SIXTEENTH + F4), (EIGTH + E4), (BREAK + R),
    (EIGTH + E4), (BREAK + R), (EIGTH + E4), (BREAK + R), (EIGTH + E4), (BREAK + R),
    (EIGTH + E4), (BREAK + R), (SIXTEENTH + E4), (SIXTEENTH + F4), (DOTTED_QUARTER + G4),
	(SIXTEENTH + F4), (SIXTEENTH + E4), (EIGTH + D4), (BREAK + R),  (EIGTH + D4), (BREAK + R), (EIGTH + D4), (BREAK + R),
    (SIXTEENTH + D4), (SIXTEENTH + E4), (DOTTED_QUARTER + F4), (SIXTEENTH + E4), (SIXTEENTH + D4), (EIGTH + E4),
    (QUARTER + C5), (EIGTH + A4), (DOTTED_EIGTH + G4), (SIXTEENTH + F4), (EIGTH + E4), (EIGTH + F4),
    (QUARTER + E4), (QUARTER + D4), (HALF + C4)
};


//define SONG_NAME "WE BARE BEARS && STEVEN UNIVERSE THEME"
//----------------------------------
#define SONG2_LENGTH   133

unsigned const char song2[SONG2_LENGTH] = {
	(SIXTEENTH + F5s), (EIGTH + G5s), (SIXTEENTH + E5), (HALF + E5), (EIGTH + R), (SIXTEENTH + B4), (EIGTH + E5),(SIXTEENTH + F5s),(SIXTEENTH + G5s),
	(SIXTEENTH + A5), // measure 2
	(SIXTEENTH + A5),(SIXTEENTH + G5s),(SIXTEENTH + F5s),(EIGTH + E5),(SIXTEENTH + F5s),(SIXTEENTH + G5s),(DOTTED_EIGTH + E5),(EIGTH + R),(SIXTEENTH + F5s),(EIGTH + G5s),(SIXTEENTH + E5), //measure 3
	(HALF + E5),(SIXTEENTH + R),(EIGTH + B4),(EIGTH + E5),(SIXTEENTH + F5s),(SIXTEENTH + G5s),(SIXTEENTH + A5), // measure 4
	(SIXTEENTH + A5),(SIXTEENTH + G5s),(SIXTEENTH + F5s),(EIGTH + E5),(SIXTEENTH + F5s),
	(SIXTEENTH + G5s),(EIGTH + E5),(EIGTH + G5s),(EIGTH + F5s),(SIXTEENTH + E5),(SIXTEENTH + C5s),(SIXTEENTH + B4), // measure 5
	(EIGTH + E5),(BREAK + R),(EIGTH + E5),(SIXTEENTH + E5),(BREAK + R),(SIXTEENTH + E5),(BREAK + R),(SIXTEENTH + E5),
	(BREAK + R),(EIGTH + D5),(BREAK + R),(EIGTH + D5),(BREAK + R),(SIXTEENTH + D5),
	(BREAK + R),(EIGTH + D5),(BREAK + R),(SIXTEENTH + D5),(BREAK + R),(SIXTEENTH + D5),(EIGTH + C5s),
	(BREAK + R),(EIGTH + C5s),(BREAK + R),(SIXTEENTH + C5s),(BREAK + R),(SIXTEENTH + C5s),(BREAK + R),
	(SIXTEENTH + C5s),(BREAK + R),(QUARTER + C5),(SIXTEENTH + R),(SIXTEENTH + F5s),(EIGTH + G5s), (SIXTEENTH + E5),
	(QUARTER + E5) , //WE BARE BEARS SONG
	(HALF + R),	//STEVEN UNIVERSE
	(HALF + B4),
	(EIGTH + G5s),(SIXTEENTH + F5s),
	(EIGTH + E5),(DOTTED_EIGTH + F5s),(DOTTED_QUARTER + E5b),(QUARTER + C5),(EIGTH + G5s),(EIGTH + F5s),(EIGTH + E5),(SIXTEENTH + E5b),
	(SIXTEENTH + E5),(DOTTED_QUARTER + E5),(SIXTEENTH + C5s),(SIXTEENTH + G5s),(EIGTH + F5s),(EIGTH + E5),(EIGTH + E5b),
	(SIXTEENTH + E5),(QUARTER + E5),(QUARTER + C5),(EIGTH + G5s),(EIGTH + F5s),(EIGTH + E5),(SIXTEENTH + F5s),
	(SIXTEENTH + E5),(DOTTED_QUARTER + E5),(QUARTER + B4),(DOTTED_EIGTH + G5s),(SIXTEENTH + F5s),(SIXTEENTH + E5),(DOTTED_QUARTER + F5s),(QUARTER + G5s),
	(QUARTER + F5s),(QUARTER + E5),(EIGTH + R),(SIXTEENTH + F4s),(BREAK + R),(SIXTEENTH + F4s),
	(SIXTEENTH + F4s), (DOTTED_QUARTER + E4), (SIXTEENTH + E4),(EIGTH + G5s),(EIGTH + F5s),(SIXTEENTH + E5),(EIGTH + F5s),
	(SIXTEENTH + E5),(QUARTER + E5),(QUARTER + C5),(DOTTED_QUARTER + G5s),(SIXTEENTH + B4),(BREAK + R),(SIXTEENTH + B4),(SIXTEENTH + B4), (DOTTED_HALF + E5), (SIXTEENTH + R)

};





void main() {
    BCSCTL1 = CALBC1_1MHZ;    // 1Mhz calibration for SMCLK clock
    DCOCTL  = CALDCO_1MHZ;

    init_P1();
    init_WDT();
    init_timerA();
    restart_song();

    _bis_SR_register(GIE+LPM0_bits);// enable general interrupts and power down CPU
}

// INITIALIZERS
/********************/

// timerA is used as the square wave freq generator
void init_timerA(void) {

    TA0CTL |= TACLR;        // reset counter
    TA0CTL = (TASSEL_2 +    // clock source = SMCLK (we manipulate this using the DCOCTL register
              ID_0 +        // clock divider = 1 (We're going to be at 1 MHz for this )
              MC_1);        // UP mode
                            // timer A interrupt off
    TA0CCTL0=0;             // compare mode, output to CCR0, no interrupt enabled
}

// WDT leads the system like a state machine controller
void init_WDT(void) {

      WDTCTL =  (WDTPW +    // password
                 WDTTMSEL + // select interval timer mode
                 WDTCNTCL + // clear watchdog timer counter
                 0 +        // SMCLK is the source
                 1);        // source/8k

    // enable the WDT interrupt
    IE1 |= WDTIE;
}

// setup Port 1 GPIO pins
void init_P1(void) {

    P1DIR |= LED;
    P1OUT |= LED;

    P1SEL |= SPEAKER;
    P1DIR |= SPEAKER;

    // Song1 button:
	P1OUT |= SONG1_BUTTON;  // pullup
	P1REN |= SONG1_BUTTON;  // enable pullup resistor
	P1IES |= SONG1_BUTTON;  // set for 1->0 transition
	P1IFG &= ~SONG1_BUTTON; // clear interrupt flag
	P1IE  |= SONG1_BUTTON;  // enable interrupt

	// Song2 button:
	P1OUT |= SONG2_BUTTON;  // pullup
	P1REN |= SONG2_BUTTON;  // enable pullup resistor
	P1IES |= SONG2_BUTTON;  // set for 1->0 transition
	P1IFG &= ~SONG2_BUTTON; // clear interrupt flag
	P1IE  |= SONG2_BUTTON;  // enable interrupt

	// Play/Pause button:
	    P1OUT |= PLAY_BUTTON;   // pullup
	    P1REN |= PLAY_BUTTON;   // enable pullup resistor
	    P1IES |= PLAY_BUTTON;   // set for 1->0 transition
	    P1IFG &= ~PLAY_BUTTON;  // clear interrupt flag
	    P1IE  |= PLAY_BUTTON;   // enable interrupt

    // Reset button:
    // 1x press restarts the song
    // 2x press changes songs

    P1OUT |= RESET_BUTTON;  // pullup
    P1REN |= RESET_BUTTON;  // enable pullup resistor
    P1IES |= RESET_BUTTON;  // set for 1->0 transition
    P1IFG &= ~RESET_BUTTON; // clear interrupt flag
    P1IE  |= RESET_BUTTON;  // enable interrupt



    // Slower Tempo button:
    P1OUT |= SLOW_BUTTON;   // pullup
    P1REN |= SLOW_BUTTON;   // enable pullup resistor
    P1IES |= SLOW_BUTTON;   // set for 1->0 transition
    P1IFG &= ~SLOW_BUTTON;  // clear interrupt flag
    P1IE  |= SLOW_BUTTON;   // enable interrupt

    // Faster Tempo button:
    P1OUT |= FAST_BUTTON;   // pullup
    P1REN |= FAST_BUTTON;   // enable pullup resistor
    P1IES |= FAST_BUTTON;   // set for 1->0 transition
    P1IFG &= ~FAST_BUTTON;  // clear interrupt flag
    P1IE  |= FAST_BUTTON;   // enable interrupt


}

// toggle play/pause
void toggle_pause(void) {

    if (system_state != 3) { //checks state

        TACCTL0 ^= OUTMOD_4;    // toggle outmod between 0 and 4 (stops timer)
        system_state = !system_state;

        if (!system_state) {
            P1OUT &= ~LED;
        }
    }
}

void decrease_tempo(void) {
    if (tempo < 11.0) { //limit how far we can go
        tempo += 0.125;
    }
}

void increase_tempo(void) {
    if(tempo > 0.125) { //same as before
        tempo -= 0.125;
    }
}

// restart current song from the beginning
void restart_song(void) {
    TA0CTL |= TACLR;        // reset clock
    TACCTL0 &= ~OUTMOD_4;   // timer is initially off
    WDT_duration = 0;
    score_counter = 0;
    tempo = DEFAULT_TEMPO;  // reset tempo to default
    isbreak = 0;
    system_state = 2;
    TA0CCR0 = notes[(curr_song[0] & NOTE_MASK)]-1; // in up mode counts from TAR=0 to TACCRO-1
    P1OUT |= LED;           //signal that we're ready
}

void select_song1(void) {
    system_state = 2;
    curr_song = song1;
    curr_song_len = SONG1_LENGTH;
    restart_song();
}

void select_song2(void) {
    system_state = 2;
    curr_song = song2;
    curr_song_len = SONG2_LENGTH;
    restart_song();
}

// interrupt handlers
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

interrupt void button_interrupt_handler(void) {

    // check which interrupt flag is set
    if (P1IFG & RESET_BUTTON) {
        P1IFG &= ~RESET_BUTTON; // reset the interrupt flag
        restart_song();
    }
    else if (P1IFG & PLAY_BUTTON) {
        P1IFG &= ~PLAY_BUTTON;
        // handle play/pause
        toggle_pause();
    }
    else if (P1IFG & SLOW_BUTTON) {
        P1IFG &= ~SLOW_BUTTON;
        // handle slower tempo here
        decrease_tempo();
    }
    else if (P1IFG & FAST_BUTTON) {
        P1IFG &= ~FAST_BUTTON;
        // handle the faster tempo here
        increase_tempo();
    }
    else if (P1IFG & SONG1_BUTTON) {
        P1IFG &= ~SONG1_BUTTON;
        select_song1();
    }
    else if (P1IFG & SONG2_BUTTON) {
        P1IFG &= ~SONG2_BUTTON;
        select_song2();
    }
}

// called every 1Mhz/8k ~=  8.2ms
interrupt void WDT_interrupt_handler(void) {

    if (system_state == 0) { // playing

        if (score_counter < curr_song_len) {

            if (!isbreak) {								// shifts the score forward 5 bits to get the durations then multiplies it by the tempo.
                if (WDT_duration >= (durations[(curr_song[score_counter] >> 5)]) * tempo) {
                    // setup next note in the score
                    WDT_duration = 0;
                    score_counter++;

                    TA0CCR0 = notes[(curr_song[score_counter] & NOTE_MASK)];

                    if (durations[(curr_song[score_counter] >> 5)] == 1) { // handles a small space between notes
                        isbreak = 1;
                    }
                    else {
                        isbreak = 0;
                    }

                }
                else {
                    WDT_duration++;	//increments the WDT_duration
                }
            }

            else {
                WDT_duration = 0;
                score_counter++;
                TA0CCR0 = notes[(curr_song[score_counter] & NOTE_MASK)];
                isbreak = 0;
            }
            //just do a small break for one cylce because he had a break

        }
        else {
            // STOP THE PRESSES WE ARE DONE
            system_state = 3;            //end state
            TACCTL0 &= ~OUTMOD_4;
            P1OUT |= LED;
        }
    }
    else if (system_state == 1) { // paused

        // flash LED
        if (WDT_flash_duration == FLASH_DURATION) {
            WDT_flash_duration = 0;
            P1OUT ^= LED;
        }
        else {
            WDT_flash_duration++;
        }
    }
}


// declare interrupt handlers
//----------------------------------
ISR_VECTOR(button_interrupt_handler,".int02")         // declare P1 interrupt handler
ISR_VECTOR(WDT_interrupt_handler,".int10")   		  // declare WDT interrupt handler
