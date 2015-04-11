#ifndef _PWM_DDR_H
#define _PWM_DDR_H
#include "Arduino.h"

#define PWM_SET_B(x) DDRB &= ~(x);
#define PWM_SET_C(x) DDRC &= ~(x);
#define PWM_SET_D(x) DDRD &= ~(x);

#define PWM_MASK_B(x) DDRB |= ~(x);
#define PWM_MASK_C(x) DDRC |= ~(x);
#define PWM_MASK_D(x) DDRD |= ~(x);

class Pwm {
    public:
        Pwm();
        void init(uint8_t ledCount);
        void pwmWriteB(uint8_t pin, uint16_t pwmPosition);
        void pwmWriteC(uint8_t pin, uint16_t pwmPosition);
        void pwmWriteD(uint8_t pin, uint16_t pwmPosition);
        void toggle();
        bool ready();
};

extern Pwm PWM;


#endif
