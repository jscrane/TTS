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

#ifndef _TTS_H_
#define _TTS_H_

class TTS {
  public:

    TTS(int pin);

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
    void setPitch(byte pitch) { defaultPitch = pitch; }

    /**
     * gets the pitch
     */
    byte getPitch(void) { return defaultPitch; }

  private:
    byte defaultPitch;
    int pin;
};

#endif
