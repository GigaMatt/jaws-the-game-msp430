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
  if (relative_position.axes[1] >= -3 && relative_position.axes[0] / 2 < relative_position.axes[1])//-6
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
Layer lower_shark_layer = {
    // Lower Shark

    (AbShape *)&right_arrow,
    {(screenWidth / 2)+10, (screenHeight / 2)}, /**< bit below & right of center +15+61*/
    {0, 0},
    {0, 0}, /* last & next pos */
    COLOR_WHITE,
    0,
};

Layer upper_shark_layer = {
    // Shark Eating Human

    (AbShape *)&right_arrow,
    {(screenWidth / 2) -25, (screenHeight / 2)-60}, /**< bit below & right of center +10,+61*/
    {0, 0},
    {0, 0}, /* last & next pos */
    COLOR_GRAY,
    &lower_shark_layer,
};

Layer border_field_layer = {
    // Border Outline
    (AbShape *)&boder_outline,
    {(screenWidth / 2), (screenHeight / 2)}, /**< center */
    {0, 0},
    {0, 0}, /* last & next pos */
    COLOR_RED,
    &upper_shark_layer};

Layer human_body_layer = {
    // Swimming Human Body
    (AbShape *)&rectangle_size_10,
    {(screenWidth / 2), (screenHeight / 2)-73},
    {0, 0},
    {0, 0}, /* last & next pos */
    COLOR_BEIGE,
    &border_field_layer,
};

Layer human_head_layer = {
    // Swimming Human Body
    (AbShape *)&rectangle_size_7,
    {(screenWidth / 2), (screenHeight / 2)}, /**< center */
    {0, 0},
    {0, 0}, /* last & next pos */
    COLOR_BLACK,
    &human_body_layer,
};


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
MovLayer ml3 = {&upper_shark_layer, {1, 0}, 0};   // Upper Shark Chases Human
MovLayer ml1 = {&human_body_layer, {1, 0}, &ml3}; // Human Swims Back & Forth
MovLayer ml4 = {&lower_shark_layer, {1,0}, &ml1};  // Bottom Shark Swims for Prey
MovLayer ml0 = {&human_head_layer, {1, 0}, &ml4};

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
        u_int color = bgColor;
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
          drawString5x7(22, 55, "JAWS: THE GAME ", COLOR_RED, COLOR_BLACK); //prev 5x7 20, 35
          buzzer_set_period(1000);
          newPos.axes[axis] += (2 * velocity);
        }
        if (velocity > 0)
        {
          drawString5x7(34, 75, "CIRCA 1975 ", COLOR_RED, COLOR_BLACK); //prev 5x7 20, 35          
        }
      } /**< if outside of fence */
    }   /**< for axis */
    ml->layer->posNext = newPos;

  } /**< for ml */
}

// System Sets Background Color
u_int bgColor = COLOR_BLACK; /**< The background color */
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

  layerInit(&human_body_layer);
  layerDraw(&human_body_layer);

  buzzer_init();

  layerGetBounds(&border_field_layer, &fieldFence);

  enableWDTInterrupts(); /**< enable periodic interrupt */
  or_sr(0x8);            /**< GIE (enable interrupts) */

  for (;;)
  {
    while (!redrawScreen)
    {                      /**< Pause CPU if screen doesn't need updating */
      P1OUT &= ~GREEN_LED; /**< Green led off witHo CPU */
      or_sr(0x10);         /**< CPU OFF */
    }
    P1OUT |= GREEN_LED; /**< Green led on when CPU on */
    redrawScreen = 0;

    // Draw Base Layer
    movLayerDraw(&ml0, &human_head_layer);
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
  //   // Play Sound

  //   buzzer_advance_frequency();
  //   decisecond_count = 0;
  // }

  static short count = 0;
  P1OUT |= GREEN_LED; /**< Green LED on when cpu on */
  count++;
  if (count == 15)
  {
    // Draw Objects on Screen

    mlAdvance(&ml1, &fieldFence);
    //mlAdvance(&ml, &fieldFence);
    if (p2sw_read())
      redrawScreen = 1;
    count = 0;
  }

  P1OUT &= ~GREEN_LED; /**< Green LED off when cpu off */
}
