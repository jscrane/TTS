#include <Arduino.h>
#include "sound.h"

#ifdef ESP32
#include <driver/dac.h>
#define CHANNEL_FOR(p) (p == 25)? DAC_CHANNEL_1: DAC_CHANNEL_2
#endif

void soundOff(int pin)
{

#if defined(__AVR__)

    switch (pin) {
#ifdef TCCR1A
    case 10:
        TCCR1A &= ~_BV(COM1B1);		// Disable PWM
        break;
    case 9:
        TCCR1A &= ~_BV(COM1A1);
        break;
#endif
#ifdef TCCR3A
    case 5:
        TCCR3A = 0;// &= ~_BV(COM3A1);
        break;
#endif
#ifdef TCCR2A
    case 3:
        TCCR2A &= ~_BV(COM2B1);
        break;
#endif
#ifdef TCCR5A
    case 46:
        TCCR5A &= ~_BV(COM5A1);
        break;
    case 45:
        TCCR5A &= ~_BV(COM5B1);
        break;
    case 44:
        TCCR5A &= ~_BV(COM5C1);
        break;
#endif
    }

#elif defined(ESP32)
    dac_output_disable(CHANNEL_FOR(pin));

#else

    analogWrite(pin, 0);
#endif
}

//https://sites.google.com/site/qeewiki/books/avr-guide/pwm-on-the-atmega328
void soundOn(int pin)
{

#if defined(__AVR__)

    switch (pin) {
#ifdef TCCR1A
    case 10:
        TCCR1A = 0;
        ICR1 = PWM_TOP;
        TCCR1B = _BV(WGM13) | _BV(CS10);
        TCNT1 = 0;
        TCCR1A |= _BV(COM1B1);
        break;
    case 9:
        TCCR1A = 0;
        ICR1 = PWM_TOP;
        TCCR1B = _BV(WGM13) | _BV(CS10);
        TCNT1 = 0;
        TCCR1A |= _BV(COM1A1);
        break;
#endif
#ifdef TCCR3A
    case 5:
        TCCR3A = 0;
        ICR3 = PWM_TOP;
        TCCR3B = _BV(WGM33) | _BV(CS30);
        TCNT3 = 0;
        TCCR3A |= _BV(COM3A1);
        break;
#endif
#ifdef TCCR2A
    case 3:
        TCCR2A = _BV(COM2B1) | _BV(WGM20);	// Non-inverted, PWM Phase Corrected
        TCCR2B = _BV(CS20) | _BV(WGM22);	// No prescaling, ditto
        OCR2B = PWM_TOP;
        TCNT2 = 0;
        break;
#endif
#ifdef TCCR5A
    case 46:
        TCCR5A = 0;
        ICR5 = PWM_TOP;
        TCCR5B = _BV(WGM13) | _BV(CS10);
        TCNT5 = 0;
        TCCR5A |= _BV(COM5A1);
        break;
    case 45:
        TCCR5A = 0;
        ICR5 = PWM_TOP;
        TCCR5B = _BV(WGM13) | _BV(CS10);
        TCNT5 = 0;
        TCCR5A |= _BV(COM5B1);
        break;
    case 44:
        TCCR5A = 0;
        ICR5 = PWM_TOP;
        TCCR5B = _BV(WGM13) | _BV(CS10);
        TCNT5 = 0;
        TCCR5A |= _BV(COM5C1);
        break;
#endif
    }
#elif defined(ESP8266)

    analogWriteFreq(12000);
    analogWriteRange(PWM_TOP);
#elif defined(ESP32)

    dac_output_enable(CHANNEL_FOR(pin));
#endif
}

void sound(int pin, byte b)
{
    // Update PWM volume
    b = (b & 15);

#if defined(__AVR__)

    // scale volume to PWM_TOP / 2
    uint16_t duty = (PWM_TOP * b) / 16 / 2;
    switch (pin) {
#ifdef TCCR1A:
    case 10:
        if (duty != OCR1B) {
            TCNT1 = 0;
            OCR1B = duty;
        }
        break;
    case 9:
        if (duty != OCR1A) {
            TCNT1 = 0;
            OCR1A = duty;
        }
        break;
#endif
#ifdef TCCR3A:
    case 5:
        if (duty != OCR3A) {
            TCNT3 = 0;
            OCR3A = duty;
        }
        break;
#endif
#ifdef TCCR2A:
    case 3:
	duty /= 256;
        if (duty != OCR2B) {
            TCNT2 = 0;
            OCR2B = duty;
        }
        break;
#endif
#ifdef TCCR5A:
    case 46:
        if (duty != OCR5B) {
            TCNT5 = 0;
            OCR5B = duty;
        }
        break;
    case 45:
        if (duty != OCR5B) {
            TCNT5 = 0;
            OCR5B = duty;
        }
        break;
    case 44:
        if (duty != OCR5B) {
            TCNT5 = 0;
            OCR5B = duty;
        }
        break;
#endif
    }
#elif defined(ESP32)

    dac_output_voltage(CHANNEL_FOR(pin), b*8);
#else

    analogWrite(pin, b*8);
#endif
}
