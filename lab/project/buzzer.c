/**
 * CS 3432 - Computer Architecture
 * Lab 02
 * By: Matthew S Montoya
 * Instructor: Daniel Cervantes
 * Purpose: To practice with C & Assembly on MSP430 hardware
 * Last Modified: 3 November 2019
 */

#include <msp430.h>
#include "libTimer.h"
#include "buzzer.h"
#define MIN_SONG_LENGTH 900
#define MAX_SONG_LENGTH 90000
static unsigned int song_length = 2500;
static signed int change_rate = 250;


void buzzer_init()
{
    /* 
       Direct timer A output "TA0.1" to P2.6.  
        According to table 21 from data sheet:
          P2SEL2.6, P2SEL2.7, anmd P2SEL.7 must be zero
          P2SEL.6 must be 1
        Also: P2.6 direction must be output
    */
    timerAUpmode();		    /* used to drive speaker */
    P2SEL2 &= ~(BIT6 | BIT7);
    P2SEL &= ~BIT7; 
    P2SEL |= BIT6;
    P2DIR = BIT6;		      /* enable output to speaker (P2.6) */
    buzzer_play_sound();  // Make Some Noise
}

// Buzzer clock = 2MHz.  (period of 1k results in 2kHz tone)
void buzzer_set_period(short cycles)
{
  CCR0 = cycles; 
  CCR1 = cycles >> 1;		/* one half cycle */
}

// Plays notes for a certain period sent to this function
void buzzer_play_sound()
{
  song_length = (song_length+change_rate);
  // Positive Change Rate
  if(change_rate>0){
    if(song_length>MAX_SONG_LENGTH){
      change_rate = -change_rate;                   // Decrement the change
      song_length = song_length+(change_rate << 1);  // Shift 
    }
  }

  // Negative Change Rate
  if(change_rate<0){
    if(song_length<MAX_SONG_LENGTH){
      change_rate = -change_rate;                   // Decrement the change
      song_length = song_length+(change_rate << 1);  // Shift 
    }
  }
  buzzer_set_period(song_length);
}