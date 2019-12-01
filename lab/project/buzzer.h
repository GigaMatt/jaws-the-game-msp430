#ifndef buzzer_included
#define buzzer_included

void buzzer_init();
void buzzer_play_sound();
void buzzer_set_period(short cycles);
void changeSharkDirection(int* x_direction, int* y_direction, int direction);
void updateSharkPosition();

#endif // included
