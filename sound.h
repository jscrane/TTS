#ifndef __SOUND_H__
#define __SOUND_H__

#define PWM_TOP (1200/2)

// Determine default PIN
#if defined(ESP32)
#define DEFAULT_PIN 25
#elif defined(ESP8266)
#define DEFAULT_PIN 16
#elif defined(__arm__) 
#define DEFAULT_PIN DAC0
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
#define DEFAULT_PIN 44
#elif defined(__AVR__) 
#define DEFAULT_PIN 3
#endif

// Define callback data type
typedef void (*tts_data_callback_type)(void *tts, int len, byte *data);

/**
 * @brief Base Output Class
 * 
 */
class BaseSound {
    public:    
        virtual ~BaseSound() {};
        virtual void soundOff() = 0;
        virtual void soundOn() = 0;
        virtual void sound(byte b) = 0;

};

/**
 * @brief Output to Pin
 * 
 */
class Sound : public BaseSound {
    public:    
        Sound();
        Sound(int pin);
        void soundOff();
        void soundOn();
        void sound(byte b);

    private:
        int pin;

};

/**
 * @brief Output to Callback
 * 
 */
class SoundCallback : public BaseSound {
    public:    
        SoundCallback(tts_data_callback_type callback, void *tts, int len=512){
            // allocate result array
            this->tts = tts;
            this->tts_callback = callback;
            this->out_data = new byte[len+1];
            this->max_length = len;
        }

        ~SoundCallback(){
            delete[] out_data;
        }

        void soundOff();
        void soundOn();
        void sound(byte b);

    private:
        void *tts;
        tts_data_callback_type tts_callback = nullptr;
        byte *out_data = nullptr;
        int max_length = 0;
        int current_length = 0;
        bool active = false;


};

#endif
