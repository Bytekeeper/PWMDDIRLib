#include <pwm_ddr.h>

void setup() {
  // Only one LED will be switched
  PWM.init(1);
  // PULL down line 12
  digitalWrite(12, LOW);
}

uint16_t counter;

void loop() {
  counter = (counter + 25) % 1024;
  delay(50);

 // Wait for the PWM back buffer to come available
  while (!PWM.ready()) ;
  // Set the PWM signal on Port B PIN 4 to "counter"
  PWM.pwmWriteB(4, counter);
  // And toggle the active PWM buffer (active new PWM settings)
  PWM.toggle();
}

