#ifndef __SOUND_H__
#define __SOUND_H__

#define PWM_TOP (1200/2)

typedef void (*data_callback_type)(int len, byte *data);

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
        Sound(int pin){
            this->pin = pin;
        }
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
        SoundCallback(data_callback_type callback, int len=512){
            // allocate result array
            this->max_length = len;
            this->callback = callback;
            this->out_data = new byte[max_length];
        }

        ~SoundCallback(){
            delete[] out_data;
        }

        void soundOff();
        void soundOn();
        void sound(byte b);

    private:
        data_callback_type callback = nullptr;
        byte *out_data = nullptr;
        int max_length = 0;
        int current_length = 0;
        bool active = false;

};

#endif
