
//******** USER CONFIG *********************

//Define the "zero" point of the AC waveform as a DC value between 0 and 1023. This needs to be correct when the sensitivity is low ( <100 )
//#define MIDPOINT 22

// Define the sensitivity of analog detection. With a very clean & consistent waveform, the sensitivity can be rased significantly.
// This value is important in determining if detected peak-to-peak values constitute a 'wave'
//#define SENSITIVITY 10

// The analog pin to use
#define ADCPin A1

// Incoming data is summed, so fetching the results every second
// will indicate speed over the previous second
// How often in mS to display the results ( 0 = print all results if possible)
#define printDelay 250

//*****************************************/

#include "AnalogFrequency.h"
#include <TM1637Display.h>

uint32_t displayTimer = 0;
uint32_t amplitude = 0, samples = 0;
TM1637Display tm1637(6, 7);

void setup() {
  Serial.begin(115200);
  setupADC(ADCPin);
  tm1637.setBrightness(100);
}

void loop() {
  static long int cnt = 0;
  static float lastFreq = 0;
  if (fAvailable() && millis() - displayTimer > printDelay) {
    displayTimer = millis();
    uint32_t frequency = getFreq(&amplitude, &samples);
    //if ((amplitude < 700 && samples < 2) || (frequency > 45)) {
    float currentSpeed = abs(frequency - lastFreq) / 19.49;
    if (frequency < 300 && frequency > 10 /*&& currentSpeed > 0.99*/) {
      tm1637.showNumberDecEx(currentSpeed * 100, 4);
      Serial.print(++cnt);
      Serial.print("\tFrequency: ");
      Serial.print(frequency);
      Serial.print("\tKM/h: ");
      Serial.print(currentSpeed);
      Serial.print("\tAmplitude: ");
      Serial.println(amplitude);
      lastFreq = frequency;
    }
  }
}
