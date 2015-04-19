#ifndef _PWM_BASE_H_
#define _PWM_BASE_H_
#include "Arduino.h"

struct PwmMask;

class PwmBase {
    public:
        PwmBase();
        void init(uint8_t ledCount);
        void pwmWriteB(uint8_t pin, uint16_t pwmPosition);
        void pwmWriteC(uint8_t pin, uint16_t pwmPosition);
        void pwmWriteD(uint8_t pin, uint16_t pwmPosition);
        void toggle();
        bool ready();
        virtual inline void set(uint8_t b, uint8_t c, uint8_t d) = 0;
        virtual inline void mask(uint8_t b, uint8_t c, uint8_t d) = 0;
};

#endif
