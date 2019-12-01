/**
 * CS 3432 - Computer Architecture
 * Lab 02
 * By: Matthew S Montoya
 * Instructor: Daniel Cervantes
 * Purpose: To practice with C & Assembly on MSP430 hardware
 * Last Modified: 02 November 2019
 */

#include <msp430.h>
#include "led.h"
#include "stateMachines.h"
#include "buzzer.h"
#include "switches.h"
#include "libTimer.h"

void main()
{
    // Timer
    configureClocks();          // Start Lib Timer

    // Initialize 
    switch_init();              // Setup Switches
    buzzer_init();              // Call Speakers
    led_init();                 // Call LEDs

    // Watchdog
    enableWDTInterrupts();      // Enable Watchdog Timer

    // Power off CPU
    or_sr(0x18);
}