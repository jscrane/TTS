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
#include "sound.h"

// acess to sound implementation, some methods are static so this needs to be static as well...


class TTS {
  public:
    /**
     * @brief Construct a new TTS object using the default pin
     * 
     */
    TTS();

    /**
     * @brief Construct a new TTS object
     * 
     * @param pin 
     */
    TTS(int pin);

    /**
     * @brief Construct a new TTS object - Uses the Callback to provide the result
     * 
     */
    TTS(data_callback_type cb, int len=512) {
      sound_api = new SoundCallback(cb,len);
      pin = -1;
      defaultPitch = 7;
    }

    /**
     * @brief Destroy the TTS object
     * 
     */
    ~TTS(){
      if(sound_api!=nullptr) {
        delete sound_api;
      }
    }


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
    BaseSound *sound_api = nullptr;

    void play(int pin, byte duration, byte soundNumber);
    byte playTone(int pin, byte soundNum, byte soundPos, char pitch1, char pitch2, byte count, byte volume);


};

#endif
