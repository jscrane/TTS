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
 * Modified for Timer5 on Arduino Mega2560 by Peter Dambrowsky.
 */

#include <Arduino.h>

#include "TTS.h"
#include "english.h"
#include "sound.h"

#define NUM_VOCAB sizeof(s_vocab)/sizeof(VOCAB)
#define NUM_PHONEME sizeof(s_phonemes)/sizeof(PHONEME)

// Random number seed
static byte seed0;
static byte seed1;
static byte seed2;

static char phonemes[128];
static char modifier[sizeof(phonemes)];	// must be same size as 'phonemes'

// Lookup user specified pitch changes
static const byte PROGMEM PitchesP[] = { 1, 2, 4, 6, 8, 10, 13, 16 };

/**
 * Find the single character 'token' in 'vocab'
 * and append its phonemes to dest[x]
 */
static int copyToken(char token, char *dest, int x, const VOCAB * vocab)
{
    for (unsigned int ph = 0; ph < NUM_VOCAB; ph++) {
	const void *txt = pgm_read_ptr(&vocab[ph].txt);
	if (pgm_read_byte(txt) == token && pgm_read_byte(txt+1) == 0) {
	    const char *src = (const char *)pgm_read_ptr(&vocab[ph].phoneme);
	    while (pgm_read_byte(src)) {
		dest[x++] = pgm_read_byte(src);
		src++;
	    }
	    break;
	}
    }
    return x;
}

static byte whitespace(char c)
{
    return (c == 0 || c == ' ' || c == ',' || c == '.' || c == '?'
	    || c == '\'' || c == '!' || c == ':' || c == '/');
}

/**
 * Enter:
 * src => English text in upper case
 * vocab => VOCAB array
 * dest => address to return result
 * return 1 if ok, or 0 if error
 */
static int textToPhonemes(const char *src, const VOCAB * vocab, char *dest)
{
    int outIndex = 0;		// Current offset into dest
    int inIndex = -1;		// Starts at -1 so that a leading space is assumed

    while (inIndex == -1 || src[inIndex]) {	// until end of text
	int maxMatch = 0;	// Max chars matched on input text
	int numOut = 0;		// Number of characters copied to output stream for the best match
	boolean endsInWhiteSpace = false;
	int maxWildcardPos = 0;

	// Get next phoneme, P2
	for (unsigned int ph = 0; ph < NUM_VOCAB; ph++) {
	    int y, x;
	    char wildcard = 0;	// modifier
	    int wildcardInPos = 0;
	    boolean hasWhiteSpace = false;
	    const void *text = pgm_read_ptr(&vocab[ph].txt);
	    const void *phon = pgm_read_ptr(&vocab[ph].phoneme);

	    for (y = 0;; y++) {
		char nextVocabChar = pgm_read_byte(text+y);
		char nextCharIn = (y + inIndex == -1) ? ' ' : src[y + inIndex];
		if (nextCharIn >= 'a' && nextCharIn <= 'z')
		    nextCharIn = nextCharIn - 'a' + 'A';

		if (nextVocabChar == '#' && nextCharIn >= 'A' && nextCharIn <= 'Z') {
		    wildcard = nextCharIn;	// The character equivalent to the '#'
		    wildcardInPos = y;
		    continue;
		}

		if (nextVocabChar == '_') {
		    // try to match against a white space
		    hasWhiteSpace = true;
		    if (whitespace(nextCharIn))
			continue;
		    y--;
		    break;
		}
		// check for end of either string
		if (nextVocabChar == 0 || nextCharIn == 0)
		    break;

		if (nextVocabChar != nextCharIn)
		    break;
	    }

	    // See if it's the longest complete match so far
	    if (y <= maxMatch || pgm_read_byte(text+y))
		continue;

	    // This is the longest complete match
	    maxMatch = y;
	    maxWildcardPos = 0;
	    x = outIndex;	// offset into phoneme return data

	    // Copy the matching phrase changing any '#' to the phoneme for the wildcard
	    for (y = 0;; y++) {
		char c = pgm_read_byte(phon+y);
		if (c == 0)
		    break;
		if (c == '#') {
		    if (pgm_read_byte(phon+y+1) == 0) {
			// replacement ends in wildcard
			maxWildcardPos = wildcardInPos;
		    } else {
			x = copyToken(wildcard, dest, x, vocab);	// Copy the phonemes for the wildcard character
		    }
		} else {
		    dest[x++] = c;
		}
	    }
	    dest[x] = 0;
	    endsInWhiteSpace = hasWhiteSpace;

	    // 14
	    numOut = x - outIndex;	// The number of bytes added
	}
	// 15 - end of vocab table

	// 16
	if (endsInWhiteSpace)
	    maxMatch--;

	// 17
	if (maxMatch == 0) {
	    //loggerP(PSTR("Mistake in SAY, no token for ")); 
	    //logger(&src[inIndex]);
	    //loggerCRLF();
	    return 0;
	}
	// 20
	outIndex += numOut;
	if (outIndex > 128 - 16) {
	    //loggerP(PSTR("Mistake in SAY, text too long\n"));
	    return 0;
	}
	// 21 
	inIndex += (maxWildcardPos > 0) ? maxWildcardPos : maxMatch;
    }
    return 1;
}

/**
 * Convert phonemes to data string
 * Enter: textp = phonemes string
 * Return: phonemes = string of sound data
 *	   modifier = 2 bytes per sound data
 */
static int phonemesToData(const char *textp, const PHONEME * phoneme)
{
    unsigned int phonemeOut = 0;	// offset into the phonemes array
    unsigned int modifierOut = 0;	// offset into the modifiers array
    unsigned int L81 = 0;	// attenuate
    unsigned int L80 = 16;

    while (*textp) {
	// P20: Get next phoneme
	boolean anyMatch = false;
	int longestMatch = 0;
	int numOut = 0;		// The number of bytes copied to the output for the longest match

	// Get next phoneme, P2
	for (unsigned int ph = 0; ph < NUM_PHONEME; ph++) {
	    int numChars;

	    // Locate start of next phoneme 
	    const void *ph_text = pgm_read_ptr(&phoneme[ph].txt);

	    // Set 'numChars' to the number of characters
	    // that we match against this phoneme
	    for (numChars = 0; textp[numChars]; numChars++) {

		// get next input character and make lower case
		char nextChar = textp[numChars];
		if (nextChar >= 'A' && nextChar <= 'Z')
		    nextChar = nextChar - 'A' + 'a';

		if (nextChar != pgm_read_byte(ph_text+numChars))
		    break;
	    }

	    // if not the longest match so far then ignore
	    if (numChars <= longestMatch)
		continue;

	    // partial phoneme match
	    if (pgm_read_byte(ph_text+numChars))
		continue;

	    // P7: we have matched the whole phoneme
	    longestMatch = numChars;

	    // Copy phoneme data to 'phonemes'
	    const void *ph_ph = pgm_read_ptr(&phoneme[ph].phoneme);
	    for (numOut = 0; pgm_read_byte(ph_ph+numOut); numOut++)
		phonemes[phonemeOut + numOut] = pgm_read_byte(ph_ph+numOut);

	    L81 = pgm_read_byte(&phoneme[ph].attenuate) + '0';
	    anyMatch = true;	// phoneme match found

	    modifier[modifierOut] = -1;
	    modifier[modifierOut + 1] = 0;

	    // Get char from text after the phoneme and test if it is a numeric
	    if (textp[longestMatch] >= '0' && textp[longestMatch] <= '9') {
		// Pitch change requested
		modifier[modifierOut] = pgm_read_byte(&PitchesP[textp[longestMatch] - '1']);
		modifier[modifierOut + 1] = L81;
		longestMatch++;
	    }
	    // P10
	    if (L81 != '0' && L81 != L80 && modifier[modifierOut] >= 0) {
		modifier[modifierOut - 2] = modifier[modifierOut];
		modifier[modifierOut - 1] = '0';
		continue;
	    }
	    // P11
	    if ((textp[longestMatch - 1] | 0x20) == 0x20) {
		// end of input string or a space
		modifier[modifierOut] = (modifierOut == 0) ? 16 : modifier[modifierOut - 2];
	    }
	}			// next phoneme

	// p13
	L80 = L81;
	if (longestMatch == 0 && !anyMatch)
	    return 0;

	// Move over the bytes we have copied to the output
	phonemeOut += numOut;

	if (phonemeOut > sizeof(phonemes) - 16)
	    return 0;

	// P16

	// Copy the modifier setting to each sound data element for this phoneme
	if (numOut > 2)
	    for (int count = 0; count != numOut; count += 2) {
		modifier[modifierOut + count + 2] = modifier[modifierOut + count];
		modifier[modifierOut + count + 3] = 0;
	    }
	modifierOut += numOut;

	//p21
	textp += longestMatch;
    }

    phonemes[phonemeOut++] = 'z';
    phonemes[phonemeOut++] = 'z';
    phonemes[phonemeOut++] = 'z';
    phonemes[phonemeOut++] = 'z';

    while (phonemeOut < sizeof(phonemes))
	phonemes[phonemeOut++] = 0;

    while (modifierOut < sizeof(modifier)) {
	modifier[modifierOut++] = -1;
	modifier[modifierOut++] = 0;
    }

    return 1;
}

static void pause(byte d)
{
    delayMicroseconds(d*6);
}

static void delay2(byte d)
{
    delayMicroseconds(d*3127);
}

/*
 * Generate a random number
 */
static byte random2(void)
{
    byte tmp = (seed0 & 0x48) + 0x38;
    seed0 <<= 1;
    if (seed1 & 0x80)
	seed0++;
    seed1 <<= 1;
    if (seed2 & 0x80)
	seed1++;
    seed2 <<= 1;
    if (tmp & 0x40)
	seed2++;
    return seed0;
}

static byte playTone(int pin, byte soundNum, byte soundPos, char pitch1, char pitch2, byte count, byte volume)
{
    const byte *soundData = &SoundData[soundNum * 0x40];
    while (count-- > 0) {
	byte s = pgm_read_byte(&soundData[soundPos & 0x3fu]);
	sound(pin, s & volume);
	pause(pitch1);
	sound(pin, (s >> 4) & volume);
	pause(pitch2);

	soundPos++;
    }
    return soundPos & 0x3fu;
}

static void play(int pin, byte duration, byte soundNumber)
{
    while (duration--)
	playTone(pin, soundNumber, random2(), 7, 7, 10, 15);
}

/******************************************************************************
 * User API
 ******************************************************************************/
TTS::TTS(int p)
{
    pin = p;
    defaultPitch = 7;
#ifdef __AVR__
    pinMode(pin, OUTPUT);
#endif
}

/*
 * Speak a string of phonemes
 */
void TTS::sayPhonemes(const char *textp)
{
    byte phonemeIn,		// offset into text
         byte2, modifierIn,	// offset into stuff in modifier
	 punctuationPitchDelta;	// change in pitch due to fullstop or question mark
    char byte1;
    char phoneme;
    const SOUND_INDEX *soundIndex;
    byte sound1Num;		// Sound data for the current phoneme
    byte sound2Num;		// Sound data for the next phoneme
    byte sound2Stop;		// Where the second sound should stop
    char pitch1;		// pitch for the first sound
    char pitch2;		// pitch for the second sound
    short i;
    byte sound1Duration;	// the duration for sound 1

    if (phonemesToData(textp, s_phonemes)) {
	// phonemes has list of sound bytes
	soundOn(pin);

	// initialise random number seed
	seed0 = 0xecu;
	seed1 = 7;
	seed2 = 0xcfu;

	// _630C
	byte1 = 0;
	punctuationPitchDelta = 0;

	// Q19
	for (phonemeIn = 0, modifierIn = 0; phonemes[phonemeIn]; phonemeIn += 2, modifierIn += 2) {
	    byte duration;	// duration from text line
	    byte soundPos;	// offset into sound data
	    byte fadeSpeed = 0;

	    phoneme = phonemes[phonemeIn];
	    if (phoneme == 'z') {
		delay2(15);
		continue;
	    } else if (phoneme == '#') {
		continue;
	    } else {

		// Collect info on sound 1
		soundIndex = &SoundIndex[phoneme - 'A'];
		sound1Num = pgm_read_byte(&soundIndex->SoundNumber);
		byte1 = pgm_read_byte(&soundIndex->byte1);
		byte2 = pgm_read_byte(&soundIndex->byte2);

		duration = phonemes[phonemeIn + 1] - '0';	// Get duration from the input line
		if (duration != 1)
		    duration <<= 1;

		duration += 6;	// scaled duration from the input line (at least 6)
		sound2Stop = 0x40 >> 1;

		pitch1 = modifier[modifierIn];
		if (modifier[modifierIn + 1] == 0 || pitch1 == -1) {
		    pitch1 = 10;
		    duration -= 6;
		} else if (modifier[modifierIn + 1] == '0' || duration == 6) {
		    duration -= 6;
		}
		// q8
		pitch2 = modifier[modifierIn + 2];
		if (modifier[modifierIn + 3] == 0 || pitch2 == -1)
		    pitch2 = 10;

		// q10
		if (byte1 < 0) {
		    sound1Num = 0;
		    random2();
		    sound2Stop = (0x40 >> 1) + 2;
		} else {
		    // is positive
		    if (byte1 == 2) {
			// 64A4
			// Make a white noise sound!
			byte volume = (duration == 6) ? 15 : 1;	// volume mask
			for (duration <<= 2; duration > 0; duration--) {
			    playTone(pin, sound1Num, random2(), 8, 12, 11, volume);
			    // Increase the volume
			    if (++volume == 16)
				volume = 15;	// full volume from now on
			}
			continue;

		    } else {
			// q11
			if (byte1)
			    delay2(25);
		    }
		}
	    }

	    // 6186
	    pitch1 += defaultPitch + punctuationPitchDelta;
	    if (pitch1 < 1)
		pitch1 = 1;

	    pitch2 += defaultPitch + punctuationPitchDelta;
	    if (pitch2 < 1)
		pitch2 = 1;

	    // get next phoneme
	    phoneme = phonemes[phonemeIn + 2];

	    if (phoneme == 0 || phoneme == 'z') {
		if (duration == 1)
		    delay2(60);
		phoneme = 'a';	// change to a pause
	    } else {
		// s6
		if (byte2 != 1)
		    byte2 = (byte2 + pgm_read_byte(&SoundIndex[phoneme - 'A'].byte2)) >> 1;

		if (byte1 < 0 || pgm_read_byte(&SoundIndex[phoneme - 'A'].byte1))
		    phoneme = 'a';	// change to a pause
	    }

	    // S10
	    sound2Num = pgm_read_byte(&SoundIndex[phoneme - 'A'].SoundNumber);

	    sound1Duration = 0x80;	// play half of sound 1
	    if (sound2Num == sound1Num)
		byte2 = duration;

	    // S11
	    if ((byte2 >> 1) == 0) {
		sound1Duration = 0xff;	// play all of sound 1
	    } else {
		// The fade speed between the two sounds
		fadeSpeed = (sound1Duration + (byte2 >> 1)) / byte2;

		if (duration == 1) {
		    sound2Stop = 0x40;	// dont play sound2
		    sound1Duration = 0xff;	// play all of sound 1
		    pitch1 = 12;
		}
	    }

	    soundPos = 0;
	    do {
		byte sound1Stop = (sound1Duration >> 2) & 0x3fu;
		byte sound1End = min(sound1Stop, sound2Stop);

		if (sound1Stop)
		    soundPos = playTone(pin, sound1Num, soundPos, pitch1, pitch1, sound1End, 15);

		// s18
		if (sound2Stop != 0x40) {
		    soundPos = playTone(pin, sound2Num, soundPos, pitch2, pitch2, sound2Stop - sound1End, 15);
		}
		// s23
		if (sound1Duration != 0xff && duration < byte2) {
		    // Fade sound1 out
		    sound1Duration -= fadeSpeed;
		    if (sound1Duration >= 0xC8)
			sound1Duration = 0;	// stop playing sound 1
		}
		// Call any additional sound
		if (byte1 == -1)
		    play(pin, 3, 30);	// make an 'f' sound
		else if (byte1 == -2)
		    play(pin, 3, 29);	// make an 's' sound
		else if (byte1 == -3)
		    play(pin, 3, 33);	// make a 'th' sound
		else if (byte1 == -4)
		    play(pin, 3, 27);	// make a 'sh' sound

	    } while (--duration);

	    // Scan ahead to find a '.' or a '?' as this will change the pitch
	    punctuationPitchDelta = 0;
	    for (i = 6; i > 0; i--) {
		char next = phonemes[phonemeIn + (i * 2)];
		if (next == 'i')
		    // found a full stop
		    punctuationPitchDelta = 6 - i;	// Lower the pitch
		else if (next == 'h')
		    // found a question mark
		    punctuationPitchDelta = i - 6;	// Raise the pitch
	    }

	    if (byte1 == 1)
		delay2(25);
	}			// next phoneme
    }
    soundOff(pin);
}

/*
 * Speak an English command line of text
 */
void TTS::sayText(const char *original)
{
    static char text[sizeof(phonemes)];

    if (textToPhonemes(original, s_vocab, text))
	sayPhonemes(text);
}
