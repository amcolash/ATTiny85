// Uses a version of TinyWireS from https://github.com/rambo/TinyWire/tree/master/TinyWireS
// I needed to manually modify TWI_TX_BUFFER_SIZE in usiTwiSlave.h from 16 -> 32 bytes
#include <TinyWireS.h>
#include <fix_fft.h>

//MUST BE POWER OF 2!
#define SAMPLES 64
#define HALF_SAMPLES SAMPLES/2
#define SCALAR 30

long start = 0;

int fft_power = (int) (log(SAMPLES) / log(2));

int i, sum, avg, count, brightness;
int8_t data[SAMPLES];
byte buff[HALF_SAMPLES];

void requestEvent() {
  // send 17 bytes (16 fft bars and 1 light level)
  
  // Only send half of the data from FFT, average each two bands together
  for (i = 0; i < HALF_SAMPLES / 2; i++) {
    TinyWireS.send(((buff[i*2] + buff[i*2 + 1]) / 2) & 0xFF);
  }

  // Send light level
  TinyWireS.send(brightness & 0xFF);
}

void setup() {
  for (i = 0; i < HALF_SAMPLES; i++) { buff[i] = 0; }
  
  TinyWireS.begin(0x1);
  TinyWireS.onRequest(requestEvent);
}

void loop() {
  // shift values to a place where they are useful for me (with 2.2k resistor)
  brightness = analogRead(3) - 480;
  
  if (brightness < 0) brightness = 0;
  if (brightness > 255) brightness = 255;

  sum = 0;

  // Try to fix issues with first band
  for (i = 0; i < 10; i++) {
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

  // Some clever manipulation to try and clean out some noisy stuff.
  // Discard 1st band and then assign 1st and 2nd band to the 2 components of 2nd band
  buff[0] = max(data[2], 1) & 0xFF;
  buff[1] = max(data[2], 1) & 0xFF;
  
  buff[2] = max(data[3], 1) & 0xFF;
  buff[3] = max(data[3], 1) & 0xFF;

  tws_delay(1);
}
