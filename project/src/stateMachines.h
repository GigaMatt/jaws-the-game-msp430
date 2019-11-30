/**
 * CS 3432 - Computer Architecture
 * Lab 03 - Christmas Toy
 * By: Matthew S Montoya
 * Instructor: Daniel Cervantes
 * Purpose: To practice with C & Assembly on MSP430 hardware
 * Last Modified: 15 November 2019
 */

// State Machine from \DimDemo
#ifndef stateMachine_included
#define stateMachine_included

void state_advance();
char toggle_red();
char toggle_green();
char toggle_red_green();

#endif // included