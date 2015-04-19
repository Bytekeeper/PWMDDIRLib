#ifndef _PWM_DDR_H_
#define _PWM_DDR_H_
#include "pwm_base.h"

class PwmDdr : public PwmBase {
    public:
        inline void set(uint8_t b, uint8_t c, uint8_t d);
        inline void mask(uint8_t b, uint8_t c, uint8_t d);
};

#endif
