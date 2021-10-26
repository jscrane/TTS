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

/**
 * @brief TTS Output Information 
 * 
 */
struct TTSInfo {
  int channels = 1;
  int sample_rate = 12000;
  int bits_per_sample = 8;
};

/**
 * @brief TTS API
 * 
 */

class TTS {
  public:
    /**
     * @brief Construct a new TTS object using the default pin
     * 
     */
    TTS(int pin);


    /**
     * @brief Construct a new TTS object - Uses the Callback to provide the result
     * 
     */
    TTS(tts_data_callback_type cb, int len=512);

    /**
     * @brief Construct a new TTS object which outputs the data to an Arduino Stream
     * 
     * @param out 
     */
    TTS(Print &out);

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

    /**
     * @brief Get additional output information
     * 
     * @return TTSInfo 
     */
    static TTSInfo getInfo() {
      TTSInfo info;
      return info;
    }

    // allow callback to access private fields
    friend void stream_data_callback(void *vtts, int len, byte *data);

  protected:
    byte defaultPitch;
    BaseSound *sound_api = nullptr;
    Print *stream_ptr = nullptr;

    void play(byte duration, byte soundNumber);
    byte playTone(byte soundNum, byte soundPos, char pitch1, char pitch2, byte count, byte volume);

    // callback to write sound data to stream
    static void stream_data_callback(void *vtts, int len, byte *data){
      TTS *tts = (TTS *) vtts;
      if (tts->stream_ptr!=nullptr && len>0 && data!=nullptr){
        tts->stream_ptr->write((const char*)data, len);
      }
    }

};




#endif
