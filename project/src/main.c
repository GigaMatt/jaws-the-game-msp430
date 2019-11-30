/**
 * CS 3432 - Computer Architecture
 * Lab 03 - Christmas Toy
 * By: Matthew S Montoya
 * Instructor: Daniel Cervantes
 * Purpose: To practice with C & Assembly on MSP430 hardware
 * Last Modified: 15 November 2019
 */

#include <msp430.h>
#include <libTimer.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include <shape.h>
#include <abCircle.h>
#include "buzzer.h"
#include "led.h"
#include "switches.h"

void main()
{
    P1DIR |= GREEN_LED;
    P1OUT |= GREEN_LED;

    /* Set Up Timer */
    configureClocks();

    /* Initialize LCD, Buzzer, LED, Shape, Layer */
    lcd_init();    // Call LCD
    buzzer_init(); // Call Speaker
    led_init();    // Call LEDs
    shapeInit();   // Call Shape
    p2sw_init(1);

    layerInit(&layer1); // Call Layer

    layerDraw(&layer1);
    layerGetBounds(&fieldLayer, &fieldFence);

    enableWDTInterrupts(); // Enable Watchdog Timer

    /* Power Off CPU */
    or_sr(0x18); /**< GIE (enable interrupts) */