#ifndef libTimer_included
#define libTimer_included

#include "clocksTimer.h"
#include "sr.h"

void configureClocks();
void enableWDTInterrupts();
void timerAUpmode();

unsigned char gert_sr();
void set_sr(), or_sr(), and_sr();


#endif // included
