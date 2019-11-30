/**
 * CS 3432 - Computer Architecture
 * Lab 03 - Christmas Toy
 * By: Matthew S Montoya
 * Instructor: Daniel Cervantes
 * Purpose: To practice with C & Assembly on MSP430 hardware
 * Last Modified: 15 November 2019
 */

#include <msp430.h>
#include "switches.h"

/* Switch on P1 (S2) */
void
    __interrupt_vec(PORT2_VECTOR) Port_2()
{
    if (P2IFG & SWITCHES)
    {
        P2IFG &= ~SWITCHES;

        // Call switches.h
        switch_interrupt_handler();
    }
}