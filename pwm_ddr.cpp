#include "pwm_ddr.h"

// #define DEBUG

const uint16_t MAXPWM = 1024;

Pwm PWM = Pwm();

// Bit masks for ports B, C and D
struct Mask {
    volatile uint8_t b;
    volatile uint8_t c;
    volatile uint8_t d;
};

struct PwmMask {
    volatile uint8_t b;
    volatile uint8_t c;
    volatile uint8_t d;
    volatile uint16_t pwm;
};

// PwmConfig defining one set of instructions for a PWM run
struct PwmConfig {
    Mask set; 
    PwmMask *mask;
};

static struct PwmConfig pwmConfig[2];
volatile struct PwmConfig *activeConfig;
volatile uint8_t activeConfigIndex;
volatile uint8_t pwmIndex;
volatile uint8_t toggleFlag;
static uint8_t numLeds;
static uint8_t irqCnt;

inline uint16_t toCounter(uint16_t pwm) {
    // Freq / Prescaler / PWMRange
    // 8000000 / 64 / 2048 = ~60Hz
    return pwm << 1;
}

inline void resetActiveConfig() {
        memset((void*) pwmConfig[activeConfigIndex].mask, 255, sizeof(PwmMask));
        memset(&pwmConfig[activeConfigIndex].set, 0, sizeof(Mask));
        pwmConfig[activeConfigIndex].mask[0].pwm = toCounter(MAXPWM);
}

inline void resetPwm() {
    if (toggleFlag) {
        toggleFlag = 0;
        resetActiveConfig();
        activeConfigIndex = 1 - activeConfigIndex;
        activeConfig = &pwmConfig[activeConfigIndex];
    }

    pwmIndex = 0;
    PWM_SET_B(activeConfig->set.b);
    PWM_SET_C(activeConfig->set.c);
    PWM_SET_D(activeConfig->set.d);
    OCR1A = activeConfig->mask[0].pwm;
    TCNT1 = 0;
}

Pwm::Pwm() {
}

void Pwm::init(uint8_t ledCount) {
    cli();
    numLeds = ledCount;
    pwmConfig[0].mask = (PwmMask*) malloc(sizeof(PwmMask) * numLeds);
    pwmConfig[1].mask = (PwmMask*) malloc(sizeof(PwmMask) * numLeds);
    TCCR1A = 0;
    TCCR1B = 0;
    // Timer1 normal mode prescaler 1/64
    TCCR1B |= (1 << CS11);
    TCCR1B |= (1 << CS10);
    TIMSK1 |= (1 << OCIE1A);

    toggleFlag = 0;
    activeConfigIndex = 1;
    resetActiveConfig();
    activeConfigIndex = 0;
    resetActiveConfig();
    resetPwm();

    sei();
}

void Pwm::toggle() {
    toggleFlag = 1;
    volatile PwmConfig *bufferedConfig = &pwmConfig[1 - activeConfigIndex];
#ifdef DEBUG
    Serial.println("--------------------------------");
    Serial.print("Active index: ");
    Serial.print(activeConfigIndex);
    Serial.print(", Active Config: ");
    Serial.println((long unsigned int) activeConfig);
    for (uint8_t i = 0; i < numLeds; i++) {
        Serial.print(i);
        Serial.print("\t| ");
        Serial.print(bufferedConfig->mask[i].b);
        Serial.print("\t| ");
        Serial.print(bufferedConfig->mask[i].c);
        Serial.print("\t| ");
        Serial.print(bufferedConfig->mask[i].d);
        Serial.print("\t| ");
        Serial.print(bufferedConfig->mask[i].pwm);
        Serial.println("\t|");
    }
#endif
}

bool Pwm::ready() {
    return !toggleFlag;
}

uint8_t determineInsertIndex(volatile PwmConfig *cfg, uint16_t pwm) {
    uint8_t i;
    for (i = 0; i < numLeds && pwm> cfg->mask[i].pwm; i++); 
    if (i < numLeds) {
        if (pwm < cfg->mask[i].pwm) {
            memmove((void*) &cfg->mask[i + 1], (void*) &cfg->mask[i], sizeof(PwmMask) * (numLeds - i - 1));
            cfg->mask[i].pwm = pwm;
        }
    }
    return i;
}

inline bool validPwm(uint16_t pwm) {
    return pwm && pwm < MAXPWM;
}

void Pwm::pwmWriteB(uint8_t pin, uint16_t pwmPosition) {
    // Switch it of at the beginning? Don't even set it
    if (!validPwm(pwmPosition)) 
        return;
    pwmPosition = toCounter(pwmPosition);
    uint8_t pinMask = 1 << pin;
    volatile PwmConfig *bufferedConfig = &pwmConfig[1 - activeConfigIndex];
    bufferedConfig->set.b |= pinMask;
    uint8_t i = determineInsertIndex(bufferedConfig, pwmPosition);
    if (i >= numLeds)
        return;
    volatile PwmMask *mask = &bufferedConfig->mask[i];
    if (pwmPosition < mask->pwm) {
        mask->b = ~pinMask;
    } else {
        mask->b &= ~pinMask;
    }
}

void Pwm::pwmWriteC(uint8_t pin, uint16_t pwmPosition) {
    // Switch it of at the beginning? Don't even set it
    if (!validPwm(pwmPosition)) 
        return;
    uint8_t pinMask = 1 << pin;
    pwmPosition = toCounter(pwmPosition);
    volatile PwmConfig *bufferedConfig = &pwmConfig[1 - activeConfigIndex];
    bufferedConfig->set.c |= pinMask;
    uint8_t i = determineInsertIndex(bufferedConfig, pwmPosition);
    if (i >= numLeds)
        return;
    volatile PwmMask *mask = &bufferedConfig->mask[i];
    if (pwmPosition < mask->pwm) {
        mask->c = ~pinMask;
    } else {
        mask->c &= ~pinMask;
    }
}

void Pwm::pwmWriteD(uint8_t pin, uint16_t pwmPosition) {
    // Switch it of at the beginning? Don't even set it
    if (!validPwm(pwmPosition)) 
        return;
    uint8_t pinMask = 1 << pin;
    pwmPosition = toCounter(pwmPosition);
    volatile PwmConfig *bufferedConfig = &pwmConfig[1 - activeConfigIndex];
    bufferedConfig->set.d |= pinMask;
    uint8_t i = determineInsertIndex(bufferedConfig, pwmPosition);
    if (i >= numLeds)
        return;
    volatile PwmMask *mask = &bufferedConfig->mask[i];
    if (pwmPosition < mask->pwm) {
        mask->d = ~pinMask;
    } else {
        mask->d &= ~pinMask;
    }
}

inline void progressPwm() {
    volatile PwmMask *mask = &activeConfig->mask[pwmIndex];
    while (TCNT1 >= mask->pwm) {
        PWM_MASK_B(mask->b);
        PWM_MASK_C(mask->c);
        PWM_MASK_D(mask->d);
        pwmIndex++;
        mask++;
    }
    uint16_t nextPwmCounterValue = pwmIndex < numLeds ? mask->pwm : toCounter(MAXPWM);
    OCR1A = nextPwmCounterValue;
}

ISR(TIMER1_COMPA_vect) {
    if (TCNT1 >= toCounter(MAXPWM)) {
        resetPwm();
        return;
    }
    progressPwm();
}
