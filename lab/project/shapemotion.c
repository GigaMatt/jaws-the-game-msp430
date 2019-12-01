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

#define GREEN_LED BIT6

/* Defined Modified Shapes for Shark*/

int abSlicedArrowCheck(const AbRArrow *shape, const Vec2 *center_position, const Vec2 *pixel)
{
  // Arrow for Shark Head

  Vec2 relative_position;
  vec2Sub(&relative_position, pixel, center_position);
  if (relative_position.axes[1] >= -6 && relative_position.axes[0] / 2 < relative_position.axes[1])
    return 0;
  else
    return abRArrowCheck(shape, center_position, pixel);
}

int abSlicedRectCheck(const AbRect *rect, const Vec2 *center_position, const Vec2 *pixel)
{
  // Rectangle for Shark Fin

  Vec2 relative_position;
  vec2Sub(&relative_position, pixel, center_position);
  if (relative_position.axes[1] <= 4 && relative_position.axes[1] / 2 < relative_position.axes[2])
    return 0;
  else
    return abRectCheck(rect, center_position, pixel);
}

//  Build Objects
AbRect rectangle_size_10 = {abRectGetBounds, abSlicedRectCheck, {10, 10}}; /**< 10x10 rectangle */
AbRect rectangle_size_7 = {abRectGetBounds, abSlicedRectCheck, {7, 7}};    /**< 7x7 human rectangle */
AbRArrow right_arrow = {abRArrowGetBounds, abSlicedArrowCheck, 30};

AbRectOutline boder_outline = {
    // Define Border Outline

    abRectOutlineGetBounds,
    abRectOutlineCheck,
    {screenWidth / 2 - 10, screenHeight / 2 - 15}};

/* Define Layers for objects */
Layer layer4 = {
    // Lower Shark

    (AbShape *)&right_arrow,
    {(screenWidth / 2) + 5, (screenHeight / 2) + 5}, /**< bit below & right of center */
    {0, 0},
    {0, 0}, /* last & next pos */
    COLOR_RED,
    0,
};

Layer layer3 = {
    // Upper Shark

    (AbShape *)&right_arrow,
    {(screenWidth / 2) + 10, (screenHeight / 2) + 61}, /**< bit below & right of center */
    {0, 0},
    {0, 0}, /* last & next pos */
    COLOR_WHITE,
    &layer4,
};

Layer border_field_layer = {
    // Border Outline
    (AbShape *)&boder_outline,
    {screenWidth / 2, (screenHeight / 2)}, /**< center */
    {0, 0},
    {0, 0}, /* last & next pos */
    COLOR_BROWN,
    &layer3};

Layer layer1 = {
    // Swimming Human
    (AbShape *)&rectangle_size_10,                 // Human as
    {screenWidth / 2 + 40, screenHeight / 2 + 51}, /**< center */
    {0, 0},
    {0, 0}, /* last & next pos */
    COLOR_BROWN,
    &border_field_layer,
};

Layer layer0 = {
    // Innocent Seal
    (AbShape *)&rectangle_size_7,
    {screenWidth / 2 + 40, screenHeight / 2 + 51}, /**< center */
    {0, 0},
    {0, 0}, /* last & next pos */
    COLOR_WHITE,
    &layer1,
};

// Layer layer0 = {
//     /** Layer with an yellow circle */
//     (AbShape *)&circle14,
//     {(screenWidth / 2) + 10, (screenHeight / 2) + 7}, /**< bit below & right of center */
//     {0, 0},
//     {0, 0}, /* last & next pos */
//     COLOR_YELLOW,
//     &layer1,
// };

////////////////////////////////////////////////////////////////////////////////
/** Moving Layer
 *  Linked list of layer references
 *  Velocity represents one iteration of change (direction & magnitude)
 */
typedef struct MovLayer_s
{
  Layer *layer;
  Vec2 velocity;
  struct MovLayer_s *next;
} MovLayer;

/* initial value of {0,0} will be overwritten */
MovLayer ml3 = {&layer3, {1, 0}, 0}; /**< not all layers move */
MovLayer ml1 = {&layer1, {0, 0}, &ml3};
MovLayer ml0 = {&layer0, {0, 0}, &ml1};
//MovLayer ml4 = { &layer4, {2,1}, &ml0 };

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
  { /* for each moving layer */
    Region bounds;
    layerGetBounds(movLayer->layer, &bounds);
    lcd_setArea(bounds.topLeft.axes[0], bounds.topLeft.axes[1],
                bounds.botRight.axes[0], bounds.botRight.axes[1]);
    for (row = bounds.topLeft.axes[1]; row <= bounds.botRight.axes[1]; row++)
    {
      for (col = bounds.topLeft.axes[0]; col <= bounds.botRight.axes[0]; col++)
      {
        Vec2 pixelPos = {col, row};
        u_int color = background_color;
        Layer *probeLayer;
        for (probeLayer = layers; probeLayer;
             probeLayer = probeLayer->next)
        { /* probe all layers, in order */
          if (abShapeCheck(probeLayer->abShape, &probeLayer->pos, &pixelPos))
          {
            color = probeLayer->color;
            break;
          } /* if probe check */
        }   // for checking all layers at col, row
        lcd_writeColor(color);
      } // for col
    }   // for row
  }     // for moving layer being updated
}

//Region fence = {{10,30}, {SHORT_EDGE_PIXELS-10, LONG_EDGE_PIXELS-10}}; /**< Create a fence region */

/* SYSTEM MOVES THE SHARKS WITHIN THE DEFINED FENCE */
void mlAdvance(MovLayer *ml, Region *fence)
{
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
          drawString5x7(20, 35, "SHARK WEEK, BABY", COLOR_RED, COLOR_BLUE); //prev 5x7
          buzzer_set_period(1000);
          newPos.axes[axis] += (2 * velocity);
        }
        if (velocity > 0)
        {
          drawString5x7(20, 35, "IT'S A BAD WEEK TO BE A SEAL", COLOR_RED, COLOR_BLUE); //prev 5x7
        }
      } /**< if outside of fence */
    }   /**< for axis */
    ml->layer->posNext = newPos;

  } /**< for ml */
}

// System Sets Background Color
u_int background_color = COLOR_GREEN; /**< The background color */
int redrawScreen = 1;                 /**< Boolean for whether screen needs to be redrawn */

Region fieldFence; /**< fence around playing field  */

void main()
{
  P1DIR |= GREEN_LED; /**< Green led on when CPU on */
  P1OUT |= GREEN_LED;

  configureClocks();
  lcd_init();
  shapeInit();
  p2sw_init(1);

  shapeInit();

  layerInit(&layer1);
  layerDraw(&layer1);

  buzzer_init();

  layerGetBounds(&fieldLayer, &fieldFence);

  enableWDTInterrupts(); /**< enable periodic interrupt */
  or_sr(0x8);            /**< GIE (enable interrupts) */

  drawString5x7(41, 6, "JAWS", COLOR_WHITE, COLOR_BLUE);
  for (;;)
  {
    while (!redrawScreen)
    {                      /**< Pause CPU if screen doesn't need updating */
      P1OUT &= ~GREEN_LED; /**< Green led off witHo CPU */
      or_sr(0x10);         /**< CPU OFF */
    }
    P1OUT |= GREEN_LED; /**< Green led on when CPU on */
    redrawScreen = 0;
    movLayerDraw(&ml0, &layer0);
  }
}

/** Watchdog timer interrupt handler. 15 interrupts/sec */
void decisecond()
{
  static char cnt = 0;
  if (++cnt > 2)
  {
    buzzer_play_sound();
    cnt = 0;
  }
}

void wdt_c_handler()
{
  static char second_count = 0, decisecond_count = 0;
  if (++decisecond_count == 25)
  {
    buzzer_play_sound();
    decisecond_count = 0;
  }

  static short count = 0;
  P1OUT |= GREEN_LED; /**< Green LED on when cpu on */
  count++;
  if (count == 15)
  {
    mlAdvance(&ml1, &fieldFence);
    if (p2sw_read())
      redrawScreen = 1;
    count = 0;
  }

  P1OUT &= ~GREEN_LED; /**< Green LED off when cpu off */
}