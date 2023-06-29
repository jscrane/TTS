/**
 * @brief Example with Callback which just prints the result 
 * 
 */
#include "TTS.h"

void data_callback(int len, byte *data){
    for (int j=0;j<len;j++){
        Serial.println(data[j]);
    }    
}

TTS tts(data_callback);

void setup(){
  Serial.begin(115200);
}

void loop(){
    tts.sayText("hallo");
    delay(5000);
}