
#include "TTS.h"

TTS tts(3);

void setup(void) {
  Serial.begin(57600);
  pinMode(LED_BUILTIN, OUTPUT);
}

char line[80];
int pos = 0;

void loop(void) {
  if (Serial.available() > 0) {
    char c = (char)Serial.read();
    if (c == '\n') {
      digitalWrite(LED_BUILTIN, HIGH);
      line[pos++] = 0;
      if (line[0] == '+') {
        int p = atoi(line+1);
        tts.setPitch(p);
      } else {
        Serial.println(line);
        tts.sayText(line);
      }
      pos = 0;
      digitalWrite(LED_BUILTIN, LOW);
    } else
      line[pos++] = c;
  }
}
