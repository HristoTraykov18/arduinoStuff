
#ifndef __ANALOGFREQ_H__
#define __ANALOGFREQ_H__

// Uncomment to start waveform detection on a LOW signal instead of HIGH
#define LOW_HI
//*******************************************************************

#if !defined(midPoint)
  #define midPoint 508      //The center or zero-point of the AC waveform expressed as a DC value between 0 & 1023
#endif
#if !defined(sensitivity)
  #define sensitivity 10    //Larger values = less sensitive (default: 5 max: < 1023-midPoint)
#endif
//*******************************************************************

volatile uint16_t reading = 0;
uint32_t varAvg = 0;
uint32_t varCnt = 0;
uint32_t ampAvg = 0;
uint32_t ampCnt = 0;
//*******************************************************************

//Keep a sum and count of the measured frequency
void saveFreq(uint32_t uS){
  varCnt++;
  varAvg += 1000000/uS;
}
//*******************************************************************

//Keep a sum and count of the measured amplitude
void saveAmp(uint16_t amplitude){
  ampCnt++;
  ampAvg += amplitude;
}
//*******************************************************************

bool fAvailable(){
  return varCnt > 0 ? true : false;
}
//*******************************************************************

//Divide the freq. sum by the count to get the average
//Further divide by two, because we are counting half-cycles
uint32_t getFreq(uint32_t *amplitude = NULL, uint32_t *samples = NULL){
  noInterrupts();
  uint32_t avg = varAvg / varCnt / 2;
  *amplitude = ampAvg / ampCnt;
  *samples = varCnt;
  varAvg = 0;  varCnt = 0;
  ampAvg = 0;  ampCnt = 0;
  interrupts();
  return avg;
}
//*******************************************************************

void setupADC(int ADCPin) {

  ADCSRA = 0;

  //Easy way to ensure the analog reference & pin are selected
  analogRead(ADCPin);

  // Enable ADC, Auto Trigger & ADC Interrupt
  ADCSRA |= _BV(ADEN) | _BV(ADATE) | _BV(ADIE);
  // Set prescale to 32: 16Mhz / 32 / 13 = ~38.5khz
  ADCSRA |= _BV(ADPS0) | _BV(ADPS2);

  // Set free running mode as ADC Auto Trigger Source
  ADCSRB = 0;

  // START up the first conversion in free run mode
  ADCSRA |= _BV(ADSC);

}
//*******************************************************************

volatile uint32_t upStartTime = 0, dnStartTime = 0;

ISR(ADC_vect) {

  reading = ADCL;
  reading |= ADCH << 8;

#if !defined LOW_HI
  if(reading >= midPoint+sensitivity && upStartTime == 0){
    upStartTime = micros();
  }else
  if(reading <= midPoint -sensitivity && upStartTime > 0){
    uint32_t tim = micros() - upStartTime;
    upStartTime = 0;
    if(tim > 50){
      saveFreq(tim);
    }
  }
#else
  if(reading <= midPoint-sensitivity && upStartTime == 0){
    upStartTime = micros();
  }else
  if(reading >= midPoint+sensitivity && upStartTime > 0){
    uint32_t tim = micros() - upStartTime;
    upStartTime = 0;
    if(tim > 50){
      saveFreq(tim);
    }
  }

#endif
  if(upStartTime > 0){
      saveAmp(reading);
  }
}
//*******************************************************************

#endif

