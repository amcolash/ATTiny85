#include <avr/wdt.h>

#define LED PB0
#define POT PB3
#define EN PB4
#define BUT PB1

int duration = 1;
int val = 0;
int debounce = 0;
int button = false;

void pulse(int dur) {
  digitalWrite(LED, HIGH);
  delay(dur);
  digitalWrite(LED, LOW);
  delay(dur);
}

void setup() {
  pinMode(LED, OUTPUT);

  pinMode(POT, INPUT);
  pinMode(EN,  INPUT);
  pinMode(BUT, INPUT);

  wdt_disable();
}

void loop() {
  // Always clear things out before checking different inputs
  digitalWrite(LED, LOW);

  // If the enable switch is off, check button
  if (!digitalRead(EN)) {
    // If push button pressed, handle and use some debouncing type logic
    // Only 1 pulse will trigger when the button is down, no matter how long - like a latch  
    if (digitalRead(BUT)) {
      if (!button && debounce == 0) {
        pulse(50);
        button = true;
      }
      debounce = 50;
    } else {
      debounce = max(0, debounce - 1);
      button = false;
    }

    // Don't continue to normal pulse, wait a moment to fool watchdog
    delay(1);
    return;
  }

  button = false;

  // If no pushbutton, do normal op reading POT and outputting accordingly. Inverted because I messed up the wiring ;)
  val = 1023 - analogRead(POT);
  pulse(val);
}
