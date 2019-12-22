#include <TM1637Display.h>

#define CLK 9
#define DIO 8

TM1637Display tm1637(CLK, DIO);

void setup() {
  Serial.begin(9600); // Default communication rate of the Bluetooth module
  
  tm1637.setBrightness(7); // 7-digit display (TM1637)

}
void loop() {
  if(Serial.available()){ // Checks whether data is comming from the serial port
    tm1637.showNumberDecEx(1); // Display time with colon to the TM1637
  }
 else {
  tm1637.showNumberDecEx(0); // Display time with colon to the TM1637
 } 
}
