#include "pwm_ddr.h"

PwmDdr PWM = PwmDdr();

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

ISR(TIMER1_COMPA_vect) {
  PWM.delegateForISR();
}
