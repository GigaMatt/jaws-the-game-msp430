/**
 * CS 3432 - Computer Architecture
 * Lab 03 - Christmas Toy
 * By: Matthew S Montoya
 * Instructor: Daniel Cervantes
 * Purpose: To practice with C & Assembly on MSP430 hardware
 * Last Modified: 15 November 2019
 */

#ifndef libTimer_included
#define libTimer_included

void configureClocks();
void enableWDTInterrupts();
void timerAUpmode();

unsigned char gert_sr();
void set_sr(), or_sr(), and_sr();

#endif // included