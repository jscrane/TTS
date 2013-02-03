#include <EtherCard.h>
#include <String.h>
#include "TTS.h"

TTS tts(3);

// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 0x74,0x69,0x69,0x2D,0x30,0x31 };

byte Ethernet::buffer[800];
static uint32_t timer;

char last_id[20];
char website[] PROGMEM = "api.supertweet.net";

// supertweet.net username:password in base64
#define KEY   "xxxxxxxxxxxxxxxxxxxxxxxxxxxxxx"

void sayIt(const char *text) {
  digitalWrite(13, HIGH);
  tts.sayText(text);
  digitalWrite(13, LOW);
}

void setup () {
  Serial.begin(57600);
  pinMode(13, OUTPUT);

  if (ether.begin(sizeof Ethernet::buffer, mymac, 10) == 0) 
    Serial.println( "Failed to access Ethernet controller");

  if (!ether.dhcpSetup())
    Serial.println("DHCP failed");

  if (!ether.dnsLookup(website))
    Serial.println("DNS failed");
    
  ether.printIp("SRV: ", ether.hisip);
  sayIt("Voice of Tic Bot");
}

void loop () {
  static byte session_id;

  ether.packetLoop(ether.packetReceive());
  if (session_id != 0) {
    const char* reply = ether.tcpReply(session_id);

    if (reply != 0) {
      const char *tok = strtok((char *)reply, "[");
      const char *id = 0, *text = 0;
      while (!id || !text) {
        const char *t = tok;
        tok = strtok(0, "\":");
        if (strcmp(t, "text") == 0)
          text = tok;
        else if (strcmp(t, "id_str") == 0)
          id = tok;
      } 
      if (strcmp(last_id, id)) {
        Serial.print(id);
        Serial.print(": ");
        Serial.println(text);
        sayIt(text);
        strcpy(last_id, id);
      }
      session_id = 0;
    }
  }
  if (millis() > timer) {
    timer = millis() + 600000;
    Stash::prepare(PSTR("GET /1.1/statuses/user_timeline.json?screen_name=TicBot&trim_user=true&include_entities=false&include_rts=false&exclude_replies=true HTTP/1.0\r\n" 
    "Host: api.supertweet.net\r\n"
    "Authorization: Basic "KEY"\r\n"
    "User-Agent: Arduino client\r\n\r\n"));

    session_id = ether.tcpSend();
  }
}
