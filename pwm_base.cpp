#include "pwm_base.h"

// #define DEBUG

const uint16_t MAXPWM = 1024;

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
volatile uint8_t activeConfigIndex;
volatile uint8_t pwmIndex;
volatile uint8_t toggleFlag;
volatile uint8_t bufferNeedsReset;
static uint8_t numLeds;
static uint8_t irqCnt;
PwmMask *currentMask;
PwmBase *instance;

inline uint16_t toCounter(uint16_t pwm) {
    // Freq / Prescaler / PWMRange
    // 8000000 / 64 / 2048 = ~60Hz
    return pwm << 1;
}

static inline void resetBuffer() {
    if (!bufferNeedsReset) {
        return;
    }
    bufferNeedsReset = 0;
    PwmConfig *backBuffer = &pwmConfig[1 - activeConfigIndex];
    memset((void*) backBuffer->mask, 255, sizeof(PwmMask));
    memset(&backBuffer->set, 0, sizeof(Mask));
    backBuffer->mask[0].pwm = toCounter(MAXPWM);
}

PwmBase::PwmBase() {
}

static inline void resetPwm();

void PwmBase::init(uint8_t ledCount) {
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
    resetBuffer();
    activeConfigIndex = 0;
    resetBuffer();
    resetPwm();

    instance = this;
    sei();
}

void PwmBase::toggle() {
    resetBuffer();
    toggleFlag = 1;
    volatile PwmConfig *bufferedConfig = &pwmConfig[1 - activeConfigIndex];
#ifdef DEBUG
    Serial.println("--------------------------------");
    Serial.print("Active index: ");
    Serial.print(activeConfigIndex);
//    Serial.print(", Active Config: ");
//    Serial.println((long unsigned int) activeConfig);
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

bool PwmBase::ready() {
    return !toggleFlag;
}

uint8_t determineInsertIndex(volatile PwmConfig *cfg, uint16_t pwm) {
    uint8_t i;
    for (i = 0; i < numLeds && pwm > cfg->mask[i].pwm; i++); 
    if (i < numLeds) {
        if (pwm < cfg->mask[i].pwm) {
            memmove((void*) &cfg->mask[i + 1], (void*) &cfg->mask[i], sizeof(PwmMask) * (numLeds - i - 1));
            cfg->mask[i].pwm = pwm;
        }
    }
    return i;
}

static inline bool validPwm(uint16_t pwm) {
    resetBuffer();
    return pwm < MAXPWM;
}

void PwmBase::pwmWriteB(uint8_t pin, uint16_t pwmPosition) {
    // Switch it of at the beginning? Don't even set it
    if (!validPwm(pwmPosition)) 
        return;
    pwmPosition = toCounter(pwmPosition);
    uint8_t pinMask = 1 << pin;
    volatile PwmConfig *bufferedConfig = &pwmConfig[1 - activeConfigIndex];
    if (pwmPosition > 0) {
        bufferedConfig->set.b |= pinMask;
    }
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

void PwmBase::pwmWriteC(uint8_t pin, uint16_t pwmPosition) {
    // Switch it of at the beginning? Don't even set it
    if (!validPwm(pwmPosition)) 
        return;
    uint8_t pinMask = 1 << pin;
    pwmPosition = toCounter(pwmPosition);
    volatile PwmConfig *bufferedConfig = &pwmConfig[1 - activeConfigIndex];
    if (pwmPosition > 0) {
        bufferedConfig->set.c |= pinMask;
    }
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

void PwmBase::pwmWriteD(uint8_t pin, uint16_t pwmPosition) {
    // Switch it of at the beginning? Don't even set it
    if (!validPwm(pwmPosition)) 
        return;
    uint8_t pinMask = 1 << pin;
    pwmPosition = toCounter(pwmPosition);
    volatile PwmConfig *bufferedConfig = &pwmConfig[1 - activeConfigIndex];
    if (pwmPosition > 0) {
        bufferedConfig->set.d |= pinMask;
    }
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

inline void resetPwm() {
    if (toggleFlag) {
        toggleFlag = 0;
        bufferNeedsReset = 1;
        activeConfigIndex = 1 - activeConfigIndex;
    }
    PwmConfig *activeConfig = &pwmConfig[activeConfigIndex];;
    currentMask = &activeConfig->mask[0];

    pwmIndex = 0;
    Mask *mask = &activeConfig->set;
    instance->set(mask->b, mask->c, mask->d);
    TCNT1 = 0;
}


inline void progressPwm(uint16_t counter) {
    while (pwmIndex < numLeds && counter >= currentMask->pwm) {
        instance->mask(currentMask->b, currentMask->c, currentMask->d);
        pwmIndex++;
        currentMask++;
    }
    uint16_t nextPwmCounterValue = pwmIndex < numLeds ? currentMask->pwm : toCounter(MAXPWM);
    OCR1A = nextPwmCounterValue > counter ? nextPwmCounterValue : counter + 1;
    TCNT1 = counter;
}

ISR(TIMER1_COMPA_vect) {
    if (!instance) {
        return;
    }
    uint16_t counter = TCNT1;
    if (counter >= toCounter(MAXPWM)) {
        resetPwm();
        counter = 0;
    }
    progressPwm(counter);
}
