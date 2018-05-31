#ifndef __SOUND_H__
#define __SOUND_H__

void soundOff(int pin);
void soundOn(int pin);
void sound(int pin, byte b);

#define PWM_TOP (1200/2)

#endif
