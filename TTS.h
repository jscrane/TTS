/** 
 * Text To Speech synthesis library 
 * Copyright (c) 2008 Clive Webster.  All rights reserved.
 *
 * Nov. 29th 2009 - Modified to work with Arduino by Gabriel Petrut:
 * The Text To Speech library uses Timer1 to generate the PWM
 * output on digital pin 10. The output signal needs to be fed
 * to an RC filter then through an amplifier to the speaker.
 * http://www.tehnorama.ro/minieric-modulul-de-control-si-sinteza-vocala/
 * 
 * Modified to allow use of different PWM pins by Stephen Crane.
 */

#ifndef TTS_h
#define TTS_h

#include <Arduino.h>
#include <inttypes.h>
#include "english.h"

// teensy DAC, set pin to enable teensy stuff, assume 8-bit DAC
#if defined(__MK20DX256__)
#define DACPIN A14   // T3.2
#elif defined(__MK64FX512__) || defined(__MK66FX1M0__)
#define DACPIN A21  // A21 or A22 T3.5 T3.6
#elif defined(__MKL26Z64__)
#define DACPIN A12  // LC
#elif defined(__SAM3X8E__)
#define DACPIN DAC0   // DUE DAC0 or DAC1
#endif

class TTS {
  public:
    /**
     * constructs a new text-to-speech
     * pin is the PWM pin on which audio is output
     * (valid values: 9, 10, 3)
     */
    TTS(int pin = 10);

    /**
     * speaks a string of (english) text
     */
    void sayText(const char *text);

    /**
     * speaks a string of phonemes
     */
    void sayPhonemes(const char *phonemes);

    /**
     * sets the pitch; higher values: lower pitch
     */
    void setPitch(byte pitch);

    /**
     * gets the pitch
     */
    byte getPitch(void);
};

#endif
