
/**
 * @brief Simple example which should work on most environments
 */

#include "TTS.h"


// TTS tts(3);  // with explicit pin
TTS tts; // with default pin

void setup(){
  Serial.begin(115200);
  // display pin
  Serial.print("Using Pin ");
  Serial.println(DEFAULT_PIN);
}

void loop(){
    tts.sayText("hallo");
    delay(5000);
}