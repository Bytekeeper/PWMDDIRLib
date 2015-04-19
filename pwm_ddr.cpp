#include "pwm_ddr.h"

inline void PwmDdr::set(uint8_t b, uint8_t c, uint8_t d) {
    DDRB &= ~b;
    DDRC &= ~c;
    DDRD &= ~d;
}

inline void PwmDdr::mask(uint8_t b, uint8_t c, uint8_t d) {
    DDRB |= ~b;
    DDRC |= ~c;
    DDRD |= ~d;
}

