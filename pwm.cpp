#include "pwm.h"

inline void Pwm::set(uint8_t b, uint8_t c, uint8_t d) {
    PORTB |= b;
    PORTC |= c;
    PORTD |= d;
}

inline void Pwm::mask(uint8_t b, uint8_t c, uint8_t d) {
    PORTB &= b;
    PORTC &= c;
    PORTD &= d;
}

