# Lab 2: Blinky-Buzzy on MSP430 Microcontroller

## Overview
The objective for this second lab is to use the MSP430 Microcontroller and our creativity to design a toy. Each MSP430 board is equipped with buttons, LED's, a speaker, and an LED screen. Utilizing these features and your knowledge of I/O, develop a program to turn the board into a toy.

![Woman-Yelling-At-Cat](/extras/3f12g7.jpg)

This toy will:

1. Generate sounds.
2. Dynamically change the LEDs that are illuminated, both brighly and dimmly.
3. Implement a state machine to maintain the state of the toy.
4. Use the four buttons on the expansion board (P2.0-3) to transition the state machine.

## How To Compile/Run
1. Using the _SystemsVM Virtual_ Machine,<br />
  a. Navigate to **/project/timerLib/**<br />
  b. Enter the command ```make```<br />
  c. Navigate to **/project/src/**<br />
  d. Connect the MSP430 Microcontroller to your computer<br />
  e. Enter the command ```make load```<br />
2. Feel free to mess around with the switches to hear different noises & trigger various LEDs.

## Dependancies
1. The 2019 version of the _SystemsVM_ Virtual Machine provided by UTEP CS 3432
2. MSP-EXP430G2ET Hardware