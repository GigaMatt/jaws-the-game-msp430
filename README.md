# Project 3: JAWS, THE GAME
## Overview
The objective for this project is to use the MSP430 Microcontroller and our creativity to design a Chritmas Toy. Each MSP430 board is equipped with buttons, an LCD, LEDs, and a speaker. Utilizing these features and our knowledge of I/O, develop a game for the LCD screen.

![Jaws-Promo-Image](/extras/jaws-promo-image.jpg)

**JAWS, THE GAME** is inspired by the 1975 Steven Spielberg thriller-film that remains a quintessential part of pop culture, through Discovery Channel's annual _Shark Week_ summer TV event. As they say during Shark Week, "it's a bad week to be a seal!" This work is a continuation of my (failed) May 2019 attempt at this project.

If your holiday gift-giving is as unnatural as I am then _Jaws, The Game_ is the perfect Christmas toy for almost any adult. With graphical sharks representative of an time long past, and physical buttons that alter the tenor of sound as you play, _Jaws, The Game_ is sure to take you back this holiday season, while ensuring your children learn about one of Hollywood's greatest films!

## Features
_JAWS, THE GAME_ will:
1. Dynamically render graphical elements that move and change directions
2. Handle colisions properly with other shapes and area boundaries
3. Produce sounds triggered by game events such that the game does not pause
4. Communicate with the player using on-screen text
5. Include a state machine written in assembly language
6. Respond to user input (from buttons)

## How To Compile & Run 
1. Using the _SystemsVM Virtual_ Machine,<br />
  a. Navigate to **/lab/timerLib/**<br />
  b. Enter the command ```make```<br />
  c. Navigate to **lab/project/**<br />
  d. Connect the MSP430 Microcontroller to your computer<br />
  e. Enter the command ```make load```<br />
2. Feel free to mess around with the switches to hear different noises & trigger various LEDs.

## Dependancies
1. The 2019 version of the _SystemsVM_ Virtual Machine provided by UTEP CS 3432
2. The MSP430 Microcontroller

## Known Issues
1. The Human swimming at the top of the screen initializes as intended but is mostly sawed off when objects begin moving.
  * Perhaps we should consider this bug a feature. Humans were generally skinnier in the 1970s

## Contributions
1. Jesus "Max" Hernandez: Teaching me how the screen is oriented.
2. Timothy "Tim" McCray: Teahing me how to work with the screen orientation for drawing shapes.
3. Andres "Andy" Ramos: Teaching me how to "create custom shapes" by slicing existing shapes.