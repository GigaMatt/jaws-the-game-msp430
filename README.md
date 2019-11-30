# Project 3: BabySharkScales LCD Game
## Overview
The objective for this project is to use the MSP430 Microcontroller and our creativity to design a Chritmas Toy. Each MSP430 board is equipped with buttons, an LCD, LEDs, and a speaker. Utilizing these features and our knowledge of I/O, develop a game for the LCD screen.

![Baby-Yoda-Meme](/extras/download.jpeg)

**BabySharkScales** is the (un)Natural Christmas toy for every good little girl and good little boy. With notes that represent musical scales and tiny sharks that roll across the screen, BabySharkScales ensures your family is no longer driven insane by the constant tune of Christmas Carols but by the same notes that every musician learns on their way to greatness!

## Features
BabySharkScales will:
1. Dynamically render graphical elements that move and change
2. Handle colisions properly with other shapes and area boundaries
3. Produce sounds triggered by game events such that the game does not pause
4. Communicate with the player using text
5. Include a state machine written in assembly language
6. Respond to user input (from buttons)

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