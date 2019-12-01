/**
 * CS 3432 - Computer Architecture
 * Lab 03 - Christmas Toy
 * By: Matthew S Montoya
 * Instructor: Daniel Cervantes
 * Purpose: To practice with C & Assembly on MSP430 hardware
 * Last Modified: 15 November 2019
 */

#include <msp430.h>
#include <lcdutils.h>
#include <lcddraw.h>
#include <p2switches.h>
#include <shape.h>
#include <abCircle.h>
#include "buzzer.h"
#include "libTimer.h"
#include "led.h"
#include "switches.h"
#include "stateMachines.h"

#define GREEN_LED BIT6

/* PART 1: DEFINE SHAPES FOR THE GAME*/

int abSlicedRectCheck(const AbRect *rect, const Vec2 *center_position, const Vec2 *pixel)
{
  // Rectangle for Shark Fin

  Vec2 relative_position;
  vec2Sub(&relative_position, pixel, center_position);
  if (relative_position.axes[1] <= 4)
  {
    if(relative_position.axes[1] / 2 < relative_position.axes[2])
    {
      return 0;
    }
  }
  else
    return abRectCheck(rect, center_position, pixel);
}

int abSlicedArrowCheck(const AbRArrow *shape, const Vec2 *center_position, const Vec2 *pixel)
{
  // Arrow for Shark Head

  Vec2 relative_position;
  vec2Sub(&relative_position, pixel, center_position);
  if (relative_position.axes[1] >= -3)
  {
    if (relative_position.axes[0] / 2 < relative_position.axes[1])
    {
      return 0;
    }
  }
  else
    return abRArrowCheck(shape, center_position, pixel);
}

//  Build Objects
AbRect rectangle_size_10 = {abRectGetBounds, abSlicedRectCheck, {7, 5}};  // 7x5 Rectangle
AbRArrow right_arrow = {abRArrowGetBounds, abSlicedArrowCheck, 25};

// Define Border Outline
AbRectOutline boder_outline = {
    abRectOutlineGetBounds,
    abRectOutlineCheck,
    {screenWidth / 2 - 10, screenHeight / 2 - 15}};

/* Define Layers for objects */
Layer center_shark_layer = {
    // Center White Shark

    (AbShape *)&right_arrow,
    {(screenWidth / 2)+10, (screenHeight / 2)}, // Positioned right of center of the screen
    {0, 0},
    {0, 0},
    COLOR_WHITE,
    0,
};

Layer upper_shark_layer = {
    // Upper Hungry Shark

    (AbShape *)&right_arrow,
    {(screenWidth / 2) -25, (screenHeight / 2)-60}, // Positioned in the upper right-hand side of the screen
    {0, 0},
    {0, 0},
    COLOR_GRAY,
    &center_shark_layer,
};

Layer border_field_layer = {
    // Border Outline

    (AbShape *)&boder_outline,
    {(screenWidth / 2), (screenHeight / 2)},  // Positioned in the center of the screen
    {0, 0},
    {0, 0},
    COLOR_RED,
    &upper_shark_layer};

Layer human_body_layer = {
    // Swimming Human Body

    (AbShape *)&rectangle_size_10,
    {(screenWidth / 2), (screenHeight / 2)-73},
    {0, 0},
    {0, 0},
    COLOR_BROWN,
    &border_field_layer,
};

Layer base_background_layer = {
    // Background Layer

    (AbShape *)&rectangle_size_10,
    {(screenWidth / 2), (screenHeight / 2)},  // Positioned in the center of the screen
    {0, 0},
    {0, 0},
    COLOR_BLACK,
    &human_body_layer,
};


/** PART 2: DEFINE MOTION LAYERS FOR THE GAME
 * Velocity is change of direction + magnitude
*/
typedef struct MovLayer_s
{
  Layer *layer;
  Vec2 velocity;
  struct MovLayer_s *next;
} MovLayer;

/* Link */
MovLayer ml3 = {&upper_shark_layer, {1, 0}, 0};   // Upper Shark Chases Human
MovLayer ml1 = {&human_body_layer, {1, 0}, &ml3}; // Human Swims Back & Forth
MovLayer ml4 = {&center_shark_layer, {1,0}, &ml1};  // Bottom Shark Swims for Prey
MovLayer ml0 = {&base_background_layer, {1, 0}, &ml4};

void movLayerDraw(MovLayer *movLayers, Layer *layers)
{
  int row, col;
  MovLayer *movLayer;

  and_sr(~8); /**< disable interrupts (GIE off) */
  for (movLayer = movLayers; movLayer; movLayer = movLayer->next)
  { /* for each moving layer */
    Layer *l = movLayer->layer;
    l->posLast = l->pos;
    l->pos = l->posNext;
  }
  or_sr(8); /**< disable interrupts (GIE on) */

  for (movLayer = movLayers; movLayer; movLayer = movLayer->next)
  {
    // For each moving layer

    Region bounds;
    layerGetBounds(movLayer->layer, &bounds);
    lcd_setArea(bounds.topLeft.axes[0], bounds.topLeft.axes[1],
                bounds.botRight.axes[0], bounds.botRight.axes[1]);
    for (row = bounds.topLeft.axes[1]; row <= bounds.botRight.axes[1]; row++)
    {
      for (col = bounds.topLeft.axes[0]; col <= bounds.botRight.axes[0]; col++)
      {
        Vec2 pixelPos = {col, row};
        u_int color = bgColor;
        Layer *probeLayer;
        for (probeLayer = layers; probeLayer;
             probeLayer = probeLayer->next)
        {
          // Probe all layers in order
          if (abShapeCheck(probeLayer->abShape, &probeLayer->pos, &pixelPos))
          {
            color = probeLayer->color;
            break;
          }
        }
        // Repaint with the background color
        lcd_writeColor(color);
      }
    }
  } 
}


/** PART 3: MOVE OBJECTS
 * Use MovLayer as defined before
*/
void mlAdvance(MovLayer *ml, Region *fence)
{
  // Move Object(s) within the defined fence

  Vec2 newPos;
  u_char axis;
  Region shapeBoundary;
  int velocity;
  for (; ml; ml = ml->next)
  {
    vec2Add(&newPos, &ml->layer->posNext, &ml->velocity);
    abShapeGetBounds(ml->layer->abShape, &newPos, &shapeBoundary);
    for (axis = 0; axis < 2; axis++)
    {
      if ((shapeBoundary.topLeft.axes[axis] < fence->topLeft.axes[axis]) ||
          (shapeBoundary.botRight.axes[axis] > fence->botRight.axes[axis]))
      {
        velocity = ml->velocity.axes[axis] = -ml->velocity.axes[axis];
        if (velocity < 0)
        {
          
          // Trigger Buzzer
          //buzzer_set_period(1000);

          drawString5x7(22, 55, "JAWS: THE GAME ", COLOR_RED, COLOR_BLACK);
          newPos.axes[axis] += (2 * velocity);
        }
        if (velocity > 0)
        {
          drawString5x7(17, 100, "PRESS ANY BUTTON ", COLOR_RED, COLOR_BLACK); //CIRCA 1975     
        }
      }
    }
    ml->layer->posNext = newPos;
  }
}

// Set Deep-Ocean-Themed Backgroud Color
u_int bgColor = COLOR_BLACK;
int redrawScreen = 1;           // Boolean for redrawing screen

Region fieldFence; /**< fence around playing field  */

void main()
{
  P1DIR |= GREEN_LED; /**< Green led on when CPU on */
  P1OUT |= GREEN_LED;

  // Timer
  configureClocks();  // Start Lib Timer

  // Initialize
  lcd_init();
  shapeInit();
  p2sw_init(1);
  shapeInit();
  switch_init();  // Setup Switches
  buzzer_init();  // Call Speakers
  led_init();     // Call LEDs
  layerInit(&human_body_layer);

  layerDraw(&human_body_layer);
  layerGetBounds(&border_field_layer, &fieldFence);

  enableWDTInterrupts();  // Enable Watchdog Timer
  or_sr(0x8);             // Power off CPU

  for (;;)
  {
    while (!redrawScreen)
    {
      // Pause CPU if screen doesn't need updating
      P1OUT &= ~GREEN_LED; // Green LED off
      or_sr(0x10);         // CPU off
    }
    P1OUT |= GREEN_LED;     // Green LED on when CPU on
    redrawScreen = 0;

    // Draw Base Layer
    movLayerDraw(&ml0, &base_background_layer);
  }
}

/** Watchdog timer interrupt handler. 15 interrupts/sec */
// void decisecond()
// {
//   static char cnt = 0;
//   if (++cnt > 2)
//   {
//     buzzer_advance_frequency();
//     cnt = 0;
//   }
// }


/* Everything Comes Together */
void wdt_c_handler()
{
  //  Transition from State to State

  // static char second_count = 0, decisecond_count = 0;
  // if (++decisecond_count == 25)
  // {

  //   buzzer_advance_frequency();
  //   decisecond_count = 0;
  // }

  static short count = 0;
  P1OUT |= GREEN_LED; /**< Green LED on when cpu on */
  count++;
  if (count == 15)
  {
    // Draw Objects on Screen & Play Sounds
    mlAdvance(&ml1, &fieldFence);

    if (p2sw_read())
      redrawScreen = 1;
    count = 0;
  }

  P1OUT &= ~GREEN_LED; /**< Green LED off when cpu off */
}
