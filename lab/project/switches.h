/**
 * CS 3432 - Computer Architecture
 * Lab 02
 * By: Matthew S Montoya
 * Instructor: Daniel Cervantes
 * Purpose: To practice with C & Assembly on MSP430 hardware
 * Last Modified: 3 November 2019
 */

#ifndef switches_included
#define switches_included

#define SW1 BIT0		/* switch2 is p1.3 */
#define SW2 BIT1
#define SW3 BIT2
#define SW4 BIT3

/* 4 switches on this board */
#define SWITCHES (SW1|SW2|SW3|SW4)	

void switch_init();
void switch_interrupt_handler();

extern short sound;
extern char switch_state_down, switch_state_changed; /* effectively boolean */

#endif // included
