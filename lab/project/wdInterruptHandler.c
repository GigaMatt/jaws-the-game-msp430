/**
 * CS 3432 - Computer Architecture
 * Lab 02
 * By: Matthew S Montoya
 * Instructor: Daniel Cervantes
 * Purpose: To practice with C & Assembly on MSP430 hardware
 * Last Modified: 02 November 2019
 */

#include <msp430.h>
#include "stateMachines.h"

void
__interrupt_vec(WDT_VECTOR) WDT(){	/* 250 interrupts/sec */
  static char blink_count = 0;
  if (++blink_count == 125) {
     sm_slow_clock();
     blink_count = 0;
  }
  sm_fast_clock();
  sm_update_led();

  led_update();
}
