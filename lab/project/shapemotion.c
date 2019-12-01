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
#define SWITCHES 0x0f  //Switches


/* PART 1: DEFINE SHAPES FOR THE GAME*/

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

//  Build Objects
AbRect rectangle_size_10 = {abRectGetBounds, abSlicedRectCheck, {7, 5}}; /**< 10x10 rectangle */
AbRArrow right_arrow = {abRArrowGetBounds, abSlicedArrowCheck, 25}; //30

AbRectOutline boder_outline = {
    // Define Border Outline

    abRectOutlineGetBounds,
    abRectOutlineCheck,
    {screenWidth / 2 - 10, screenHeight / 2 - 15}};

/* Define Layers for objects */
Layer center_shark_layer = {
    // Center White Shark

    (AbShape *)&right_arrow,
    {(screenWidth / 2)+10, (screenHeight / 2)},
    {0, 0},
    {0, 0}, /* last & next pos */
    COLOR_WHITE,
    0,
};

Layer upper_shark_layer = {
    // Upper Hungry Shark

    (AbShape *)&right_arrow,
    {(screenWidth / 2) -25, (screenHeight / 2)-60},
    {0, 0},
    {0, 0}, /* last & next pos */
    COLOR_GRAY,
    &center_shark_layer,
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
    COLOR_BROWN,
    &border_field_layer,
};

Layer human_head_layer = {
    // Swimming Human Body
    (AbShape *)&rectangle_size_10,
    {(screenWidth / 2), (screenHeight / 2)}, /**< center */
    {0, 0},
    {0, 0}, /* last & next pos */
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
MovLayer ml3 = {&upper_shark_layer, {2, 0}, 0};   // Upper Shark Chases Human
MovLayer ml1 = {&human_body_layer, {2, 0}, &ml3}; // Human Swims Back & Forth
MovLayer ml4 = {&center_shark_layer, {1,0}, &ml1};  // Bottom Shark Swims for Prey
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
    // Collisions
    
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
          
          //Trigger Buzzer
          //buzzer_play_sound();

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

// Set Dark, Christmas-Themed Backgroud Color
u_int bgColor = COLOR_BLACK;
int redrawScreen = 1;           // Boolean for redrawing screen

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
  p2sw_init(SWITCHES);

  buzzer_init();

  layerGetBounds(&border_field_layer, &fieldFence);

  enableWDTInterrupts(); /**< enable periodic interrupt */
  or_sr(0x8);            /**< GIE (enable interrupts) */

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

  //   buzzer_advance_frequency();
  //   decisecond_count = 0;
  // }

  // static short count = 0;
  // P1OUT |= GREEN_LED; /**< Green LED on when cpu on */
  // count++;
  // if (count == 15)
  // {
    // Draw Objects on Screen & Play Sounds
  //   mlAdvance(&ml1, &fieldFence);
  //   if (p2sw_read())
  //     redrawScreen = 1;
  //   count = 0;
  // }
  // P1OUT &= ~GREEN_LED; /**< Green LED off when cpu off */


	char buttonPressed = p2sw_read();
    
  static short count = 0;
  P1OUT |= GREEN_LED;		      /**< Green LED on when cpu on */
  count ++;
  if (count == 50) {

	  switch (buttonPressed) {	
          
		case 1:
      //led_state = 0;
      //sound = 1000;
      //period = 10;
      //buzzer_play_sound();
      //led_changed = 1;
      //led_advance();
      //led_update();
      //switch_state_down = 1;
			// mlAdvance(&moveLeft, &fieldFence, &mlfrog);
			// movLayerDraw(&moveLeft, &frog);
			break;
		case 2:
			// mlAdvance(&moveRight, &fieldFence, &mlfrog);
			// movLayerDraw(&moveRight, &frog);
      //buzzer_play_sound();
			break;
		case 4:
			// mlAdvance(&moveUp, &fieldFence, &mlfrog);
			// movLayerDraw(&moveUp, &frog);
      //buzzer_play_sound();
			break;
		case 8:
			// mlAdvance(&moveDown, &fieldFence, &mlfrog);
			// movLayerDraw(&moveDown, &frog);
      //buzzer_play_sound();
			break;
	  }
    mlAdvance(&ml1, &fieldFence);
    if (p2sw_read())
      redrawScreen = 1;
    count = 0;
  }
  P1OUT &= ~GREEN_LED; /**< Green LED off when cpu off */
}

//Changes the direction the pacman is moving based on what button was pressed
void updateSharkPosition(){
  MovLayer* shark_move_layer = &ml4; //Center Shark's MovLayer`// Could also me ml4? or mml0?
  Layer* shark_layer = &center_shark_layer; 
  Vec2 newposition;
  int x_direction, y_direction;
  vec2Add(&newposition, &(shark_layer->posNext), &(shark_move_layer->velocity));
  int direction = 0;

  //direction pacman will go to depending on whats pressed
  switch((P2IFG & (SWITCHES))){
  case BIT0: direction = 1; 
  break; //Button 1
  case BIT1: direction = 2; 
  break; //Button 2
  case BIT2: direction = 3; 
  break; //Button 3
  case BIT3: direction = 4; 
  break; //Button 4
  default: return;
  }

  changeVelocity(&x_direction, &y_direction, direction); //Update new velocity based on direction
  shark_move_layer->velocity.axes[0] = x_direction; //Update velocity of the Object
  shark_move_layer->velocity.axes[1] = y_direction; //Update velocity of the Object
  shark_layer->posNext = newposition;
  P2IFG = 0;
}
// void
// __interrupt_vec(PORT2_VECTOR) Port_2(){
//   updateSharkPosition();
// }


//velocity x and velocity y is changed to match the direction
void changeVelocity(int* x_direction, int* y_direction, int direction){
  switch( direction ){
  case 1: (*x_direction) = 0; (*y_direction) = -3; 
  break; //up
  case 2: (*x_direction) = 0; (*y_direction) = 3; 
  break; //down
  case 3: (*x_direction) = -3; (*y_direction) = 0; 
  break; //right
  case 4: (*x_direction) = 3; (*y_direction) = 0;
  break; //left
  default: (*x_direction) = 5; (*y_direction) = 5;
  }
}

