#include <TinyWireS.h>
#include <fix_fft.h>

//MUST BE POWER OF 2!
#define SAMPLES 64
#define HALF_SAMPLES SAMPLES/2
#define SCALAR 30

long start = 0;

int fft_power = (int) (log(SAMPLES) / log(2));

int i, sum, avg, count;
int8_t data[SAMPLES];
byte buff[HALF_SAMPLES];

void requestEvent() {
  // send 17 bytes (16 fft bars and 1 light level)
  
  // Only send half of the data from FFT, average each two bands together
  for (i = 0; i < HALF_SAMPLES / 2; i++) {
    TinyWireS.send(((buff[i*2] + buff[i*2 + 1]) / 2) & 0xFF);
  }

  // Send light level
  TinyWireS.send(123 & 0xFF);
}

void setup() {
  for (i = 0; i < HALF_SAMPLES; i++) { buff[i] = 0; }
  
  TinyWireS.begin(0x1);
  TinyWireS.onRequest(requestEvent);
}

void loop() {
  sum = 0;

  // Try to fix issues with first band
  for (i = 0; i < 5; i++) {
    analogRead(2);
  }
  
  for (i = 0; i < SAMPLES; i++) {
    data[i] = ((analogRead(2) * SCALAR) >> 2) - 128;  // convert to 8-bit value
    sum += data[i];
  }

  avg = sum / SAMPLES;
  for (i = 0; i < SAMPLES; i++){
    data[i] -= avg;
  }

  fix_fftr(data, fft_power, 0);

  // fill i2c buffer with fft data
  for(i = 0; i < HALF_SAMPLES; i++) {
    buff[i] = max(data[i], 1) & 0xFF;
  }

  tws_delay(1);
}
