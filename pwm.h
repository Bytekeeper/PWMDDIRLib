#ifndef _PWM_H_
#define _PWM_H_
#include "pwm_base.h"

class Pwm : public PwmBase {
    protected:
        inline void set(uint8_t b, uint8_t c, uint8_t d);
        inline void mask(uint8_t b, uint8_t c, uint8_t d);
};

extern Pwm PWM;


#endif
