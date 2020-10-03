// Reflectance transformation imaging software
// Developed by Hristo Traykov, NEON.BG (Sofia)

#include <FastLED.h>
#include <LiquidCrystal_I2C.h>

#define NUM_LEDS               27 // LED pixels in the strip
#define COUNTER_OFF            -1 // Does not show current pixel number on the display
#define HIGH_INPUT_DELAY      125
#define HIGH_INPUT_DURATION   125

#define CLOCK_PIN               2 // Data pin for the LED strip | GREY
#define DATA_PIN                3 // Data pin for the LED strip | PRPL
#define CAMERA_PIN              4 // Makes the camera take a picture
#define INIT_FAST_BUTTON_PIN    5 // If clicked LEDs change 26 times and 26 pictures are taken
#define INIT_MEDIUM_BUTTON_PIN  6 // If clicked LEDs change 39 times and 39 pictures are taken
#define INIT_FULL_BUTTON_PIN    7 // If clicked LEDs change 78 times and 78 pictures are taken
#define INC_WAIT_BUTTON_PIN     8 // Increases wait time
#define DEC_WAIT_BUTTON_PIN     9 // Decreases wait time

int waitTime = 1000; // Time to wait between taking pictures
char currentFlashMode[10] = "Idle";
bool stopEarly = false;
struct flashMode { // LEDs flashing mode posibilities
  char Fast[10] = "Fast";
  char Medium[10] = "Medium";
  char Full[10] = "Full";
  char Idle[10] = "Idle";
} flashMode;

CRGB leds[NUM_LEDS]; // Define the array of LEDs
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Set the LCD address to 0x27 for a 16 chars and 2 line display

void setup() {
  Serial.begin(115200); // Start serial monitor at the specified baud rate
  
  // LCD initialization
  lcd.init();
  lcd.backlight();
  lcd.clear();
  // lcd.setCursor(0, 0); // Column, row <- Reminder

  // Set buttonPins as inputs
  pinMode(INIT_FAST_BUTTON_PIN, INPUT_PULLUP);
  pinMode(INIT_MEDIUM_BUTTON_PIN, INPUT_PULLUP);
  pinMode(INIT_FULL_BUTTON_PIN, INPUT_PULLUP);
  pinMode(INC_WAIT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(DEC_WAIT_BUTTON_PIN, INPUT_PULLUP);
  pinMode(CAMERA_PIN, OUTPUT);  // Set camera pin as output
  digitalWrite(CAMERA_PIN, HIGH);
  //digitalWrite(CAMERA_PIN, LOW);
  //FastLED.addLeds<WS2811, DATA_PIN, BRG>(leds, NUM_LEDS); // WS2811 needs only data pin
  FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS); // WS2801 needs data and clock pins
  TurnLEDs_OFF();
  RTI_Screen();
  UpdateDisplayInfo(COUNTER_OFF);
}

void loop() {
  ButtonClickHandler();
}

// ___________________________________________________ Additional functions ___________________________________________________ //

// ------------------- Main handler function that calls the other functions depending on the pressed button ------------------- //
void ButtonClickHandler() {
  if (!digitalRead(INIT_FAST_BUTTON_PIN)) { // Run fast loop when the button is pressed
    Serial.println(F("Button fast"));
    strncpy(currentFlashMode, flashMode.Fast, sizeof(flashMode.Fast)); // Set the current flash mode to fast
    TakePictures(INIT_FAST_BUTTON_PIN);
    TurnLEDs_OFF();
  }
  
  if (!digitalRead(INIT_MEDIUM_BUTTON_PIN)) { // Run medium speed loop when the button is pressed
    Serial.println(F("Button medium"));
    strncpy(currentFlashMode, flashMode.Medium, sizeof(flashMode.Medium)); // Set the current flash mode to medium
    TakePictures(INIT_MEDIUM_BUTTON_PIN);
    TurnLEDs_OFF();
  }
  
  if (!digitalRead(INIT_FULL_BUTTON_PIN)) { // Run slow(full) loop when the button is pressed
    Serial.println(F("Button full"));
    strncpy(currentFlashMode, flashMode.Full, sizeof(flashMode.Full)); // Set the current flash mode to full
    TakePictures(INIT_FULL_BUTTON_PIN);
    TurnLEDs_OFF();
  }
  
  if (!digitalRead(INC_WAIT_BUTTON_PIN)) { // Increase the delay between pixels shining when the button is pressed
    bool waitTimeIncreased = false;
    while (!digitalRead(INC_WAIT_BUTTON_PIN)) {
      Serial.println(F("Button not released!"));
      if (waitTime < 5000) {
        waitTime += 250;
        Serial.print(F("Changed waitTime to "));
        Serial.println(waitTime);
        waitTimeIncreased = true;
        UpdateDisplayInfo(COUNTER_OFF);
      }
      delay(400);
    }
    delay(100);
    if (waitTime < 5000 && !waitTimeIncreased) {
      waitTime += 250;
      Serial.print(F("Changed waitTime to "));
      Serial.println(waitTime);
      UpdateDisplayInfo(COUNTER_OFF);
    }
  }
  
  if (!digitalRead(DEC_WAIT_BUTTON_PIN)) { // Decrease the delay between pixels shining when the button is pressed
    bool waitTimeDecreased = false;
    while (!digitalRead(DEC_WAIT_BUTTON_PIN)) {
      Serial.println(F("Button not released!"));
      if (waitTime > 250) {
        waitTime -= 250;
        Serial.print(F("Changed waitTime to "));
        Serial.println(waitTime);
        waitTimeDecreased = true;
        UpdateDisplayInfo(COUNTER_OFF);
      }
      delay(400);
    }
    delay(100);
    if (waitTime > 250 && !waitTimeDecreased) {
      waitTime -= 250;
      Serial.print(F("Changed waitTime to "));
      Serial.println(waitTime);
      UpdateDisplayInfo(COUNTER_OFF);
    }
  }
}

// ------------------------ Check if a button is pressed while the 'TakePictures' function is running ------------------------- //
void CheckStopClick(int pressedButtonPin) { // 
  unsigned long startMillis = millis();
  while (millis() - startMillis <= waitTime) {
    if (!digitalRead(pressedButtonPin)) {
      while (!digitalRead(pressedButtonPin)) {
        Serial.println(F("Button not released!"));
        delay(400);
      }
      Serial.println(F("Stop early"));
      digitalWrite(CAMERA_PIN, HIGH);
      stopEarly = true;
      delay(100);
      break;
    }
  }
}

// ------------------------------------------------ Shows 'RTI Dome' on the LCD ----------------------------------------------- //
void RTI_Screen() {
  lcd.setCursor(4, 0); // Column, row
  lcd.printstr("RTI DOME");
  delay(2000);
  lcd.clear();
}

// ------------------ Specifically show pixels depending on the button pressed, after the button is released ------------------ //
void TakePictures(int pressedButtonPin) {
  while (!digitalRead(pressedButtonPin)) {
    Serial.println(F("Button not released!"));
    delay(250);
  }
  delay(100);

  // Every if statement does a loop through all pixels connected.
  // 1 color channel of each pixel is used per loop cicle
  int initIncr, incr; // Increment value used in the loops and to output current pixel in the LCD
  if (pressedButtonPin == INIT_FAST_BUTTON_PIN) { // Fast looping
    initIncr = 3;
    incr = initIncr;
    for (int i = 0; i < NUM_LEDS && !stopEarly; i += initIncr) {
      if (i > 2) // Turn off the last pixel
        leds[i - initIncr] = CRGB::Black;
      for (int j = 0; j < 3; j++) { // In fast mode totally 27 channels are shown
        if (j == 0)
          leds[i] = CRGB::Red;
        else if (j == 1)
          leds[i] = CRGB::Green;
        else
          leds[i] = CRGB::Blue;
        FastLED.show();
        UpdateDisplayInfo(incr);
        incr += initIncr;
        delay(HIGH_INPUT_DELAY);
        
        unsigned long startMillis = millis();

        while (millis() - startMillis < HIGH_INPUT_DURATION)
          digitalWrite(CAMERA_PIN, LOW);
          
        CheckStopClick(pressedButtonPin);
        if (stopEarly)
          break;
        digitalWrite(CAMERA_PIN, HIGH);
      }
    }
  }

  else if (pressedButtonPin == INIT_MEDIUM_BUTTON_PIN) { // Medium speed looping
    initIncr = 2;
    incr = initIncr;
    for (int i = 0; i < NUM_LEDS && !stopEarly; i += initIncr) {
      if (i > 1) // Turn off the last pixel
        leds[i - initIncr] = CRGB::Black;
      for (int j = 0; j < 3; j++) { // In fast mode totally 39 channels are shown
        if (j == 0)
          leds[i] = CRGB::Red;
        else if (j == 1)
          leds[i] = CRGB::Green;
        else
          leds[i] = CRGB::Blue;
        FastLED.show();
        UpdateDisplayInfo(incr);
        incr += initIncr;
        delay(HIGH_INPUT_DELAY);
        
        unsigned long startMillis = millis();
        while (millis() - startMillis < HIGH_INPUT_DURATION)
          digitalWrite(CAMERA_PIN, LOW);
          
        CheckStopClick(pressedButtonPin);
        if (stopEarly)
          break;
        digitalWrite(CAMERA_PIN, HIGH);
      }
    }
  }

  else { // Slow speed looping
    initIncr = 1;
    incr = initIncr;
    for (int i = 0; i < NUM_LEDS && !stopEarly; i += initIncr) {
      if (i > 0) // Turn off the last pixel
        leds[i - initIncr] = CRGB::Black;
      for (int j = 0; j < 3; j++) { // In fast mode totally 81 channels are shown
        if (j == 0)
          leds[i] = CRGB::Red;
        else if (j == 1)
          leds[i] = CRGB::Green;
        else
          leds[i] = CRGB::Blue;
        FastLED.show();
        UpdateDisplayInfo(incr);
        incr += initIncr;
        delay(HIGH_INPUT_DELAY);
        
        unsigned long startMillis = millis();

        while (millis() - startMillis < HIGH_INPUT_DURATION)
          digitalWrite(CAMERA_PIN, LOW);
          
        CheckStopClick(pressedButtonPin);
        if (stopEarly)
          break;
        digitalWrite(CAMERA_PIN, HIGH);
      }
    }
  }
  
  strncpy(currentFlashMode, flashMode.Idle, sizeof(currentFlashMode));
  UpdateDisplayInfo(COUNTER_OFF);
  stopEarly = false;
}

// ------------------------------------------- Makes sure all pixels are turned off ------------------------------------------- //
void TurnLEDs_OFF() {
  for (int i = 0; i < NUM_LEDS; i++) {
    leds[i] = CRGB::Black;
    FastLED.show();
  }
  Serial.println(F("LEDs turned OFF"));
}

// -------------------------------------------- Updates the information on the LCD -------------------------------------------- //
void UpdateDisplayInfo(int currentPixel) {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.printstr("Delay");
  char waitTimeStr[20], tempArr[20], currentPixelStr[20];
  if (currentPixel != COUNTER_OFF) {
    lcd.setCursor(7, 0);
    lcd.printstr("ID");
    lcd.setCursor(7, 1);
    sprintf(tempArr, "%u", currentPixel);
    strncpy(currentPixelStr, tempArr, sizeof(tempArr));
    lcd.printstr(currentPixelStr);
  }
  lcd.setCursor(0, 1);
  sprintf(tempArr, "%u", waitTime / 1000);
  strncpy(waitTimeStr, tempArr, sizeof(tempArr));
  strncat(waitTimeStr, ".", sizeof("."));
  sprintf(tempArr, "%u", (waitTime / 10) % 100);
  strncat(waitTimeStr, tempArr, sizeof(tempArr));
  strncat(waitTimeStr, "sec", sizeof("sec"));
  lcd.printstr(waitTimeStr);
  
  lcd.setCursor(11, 0);
  lcd.printstr("Mode");
  if ((String)currentFlashMode == flashMode.Medium)
    lcd.setCursor(10, 1);
  else
    lcd.setCursor(11, 1);
  lcd.printstr(currentFlashMode);
  Serial.println(F("Display updated"));
}
