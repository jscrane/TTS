How PWM is used for TTS
-----------------------

We'll analyze the code for speech output on Arduino pin 9. This is
hardware pin 15 (known here as OC1A).

A good introduction to PWM can be found 
[here](https://www.arduino.cc/en/Tutorial/SecretsOfArduinoPWM). This
[guide](https://sites.google.com/site/qeewiki/books/avr-guide/pwm-on-the-atmega328)
is also useful. 

At it's most basic however, a PWM waveform is a square-wave with a varying 
duty-cycle. The duty-cycle is the ratio of time spent high to the total
cycle time. The AVR's built-in timers are used to determine both the
frequency and the duty-cycle.

Sound is initialised in soundOn():

```c++
	TCCR1A = 0;
	ICR1 = PWM_TOP;
	TCCR1B = _BV(WGM13) | _BV(CS10);
	TCNT1 = 0;
	TCCR1A |= _BV(COM1A1);
```

In this mode, known as "PWM, Phase and Frequency corrected", Timer-1
counts up from 0 to PWM_TOP and then back down to 0 again.

The PWM frequency in this mode is given by the formula:

> freq = Clock / (2 x Prescaler x TOP)

Here Clock = 16e6, Prescaler = 1 (set by CS10 above) and TOP = 1200/2.
This gives a fixed frequency of about 13kHz.

The PWM duty cycle is changed in sound():

```c++
	if (duty != OCR1A) {
		TCNT1 = 0;
		OCR1A = duty;
	}
```

Here the OCR1A value is used to set the value at which Timer-1's counter
causes the output on OC1A to flip:
- while it's counting up from 0 (BOTTOM) to OCR1A, OC1A is high,
- when it reaches OCR1A it sets OC1A low and counts to TOP (ICR1),
- when it reaches TOP it starts counting down again,
- when it reaches OCR1A again, it sets OC1A high and counts to BOTTOM (0).
