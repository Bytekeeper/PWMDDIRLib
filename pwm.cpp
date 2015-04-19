#include "pwm.h"

Pwm PWM = Pwm();

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

ISR(TIMER1_COMPA_vect) {
  PWM.delegateForISR();
}
