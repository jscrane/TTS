# TTS

## Text-to-Speech for Arduino

- requires an amplifier on the PWM output pin, e.g., [this one](http://www.tinyurl.com/magicmouth/)
- original implementation by Clive Webster in [Webbotlib](http://webbot.org.uk/iPoint/30.page)
- ported to Arduino by [Gabriel Petrut](http://www.tehnorama.ro/minieric-modulul-de-control-si-sinteza-vocala/)
- Stephen Crane modified it to use pins other than pin 10 with the help of [this tutorial](https://sites.google.com/site/qeewiki/books/avr-guide/pwm-on-the-atmega328)
- see [blog articles](http://programmablehardware.blogspot.ie/search/label/tts)

![alt tag](images/Arduino-LM386.png)

## Text-to-Speech for ARM with DAC

- add support for ARM processors with DAC
- PWM filter not required with DAC, but still need audio amp and speaker. DACs can source only about 1 ma
- tested on Teensy 3.1, 3.5, 3.6, LC, and DUE
- Teensy [forum](https://forum.pjrc.com/threads/44587-TTS-(Text-to-Speech)-Library-Port)
- separate port/hack for MBED ARM with DAC [repository](https://developer.mbed.org/users/manitou/code/tts/)
- also added AVR support for Leonardo pin 5 (timer 3)
