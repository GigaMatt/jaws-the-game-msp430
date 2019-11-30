/**
 * CS 3432 - Computer Architecture
 * Lab 03 - Christmas Toy
 * By: Matthew S Montoya
 * Instructor: Daniel Cervantes
 * Purpose: To practice with C & Assembly on MSP430 hardware
 * Last Modified: 15 November 2019
 */

#ifndef buzzer_included
#define buzzer_included

void buzzer_init();
void buzzer_set_period(short cycles);
void buzzer_play_sound();

#endif // included