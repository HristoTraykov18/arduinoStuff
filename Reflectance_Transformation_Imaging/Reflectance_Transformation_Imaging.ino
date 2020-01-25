// Reflectance transformation imaging software
// Developed by Hristo Traykov, NEON.BG (Sofia)

#include <FastLED.h>
#include <LiquidCrystal_I2C.h>

#define NUM_LEDS               27 // LED pixels in the strip

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
  //FastLED.addLeds<WS2811, DATA_PIN, BRG>(leds, NUM_LEDS); // WS2811 needs only data pin
  FastLED.addLeds<WS2801, DATA_PIN, CLOCK_PIN, RGB>(leds, NUM_LEDS); // WS2801 needs data and clock pins
  TurnLEDs_OFF();
  RTI_Screen();
  UpdateDisplayInfo();
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
        UpdateDisplayInfo();
      }
      delay(400);
    }
    delay(100);
    if (waitTime < 5000 && !waitTimeIncreased) {
      waitTime += 250;
      Serial.print(F("Changed waitTime to "));
      Serial.println(waitTime);
      UpdateDisplayInfo();
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
        UpdateDisplayInfo();
      }
      delay(400);
    }
    delay(100);
    if (waitTime > 250 && !waitTimeDecreased) {
      waitTime -= 250;
      Serial.print(F("Changed waitTime to "));
      Serial.println(waitTime);
      UpdateDisplayInfo();
    }
  }
}

// ------------------------ Check if a button is pressed while the 'TakePictures' function is running ------------------------- //
void CheckStopClick(int pressedButtonPin) { // 
  unsigned long startMillis = millis();
  while (millis() - startMillis <= waitTime) {
    if (!digitalRead(pressedButtonPin)) {
      Serial.println(F("Stop early"));
      stopEarly = true;
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
  UpdateDisplayInfo();
  
  // Every if statement does a loop through all pixels connected.
  // 1 color channel of each pixel is used per loop cicle
  if (pressedButtonPin == INIT_FAST_BUTTON_PIN) { // Fast looping
    for (int i = 0; i < NUM_LEDS; i += 3) {
      if (i > 2) // Turn off the last pixel
        leds[i - 3] = CRGB::Black;
      for (int j = 0; j < 3; j++) { // In fast mode totally 27 channels are shown
        if (j == 0)
          leds[i] = CRGB::Red;
        else if (j == 1)
          leds[i] = CRGB::Green;
        else
          leds[i] = CRGB::Blue;
        FastLED.show();
        digitalWrite(CAMERA_PIN, HIGH);
        CheckStopClick(pressedButtonPin);
        if (stopEarly) {
          stopEarly = false;
          break;
        }
        digitalWrite(CAMERA_PIN, LOW);
      }
    }
  }

  else if (pressedButtonPin == INIT_MEDIUM_BUTTON_PIN) { // Medium speed looping
    for (int i = 0; i < NUM_LEDS; i += 2) {
      if (i > 1) // Turn off the last pixel
        leds[i - 2] = CRGB::Black;
      for (int j = 0; j < 3; j++) { // In fast mode totally 39 channels are shown
        if (j == 0)
          leds[i] = CRGB::Red;
        else if (j == 1)
          leds[i] = CRGB::Green;
        else
          leds[i] = CRGB::Blue;
        FastLED.show();
        digitalWrite(CAMERA_PIN, HIGH);
        CheckStopClick(pressedButtonPin);
        if (stopEarly) {
          stopEarly = false;
          break;
        }
        digitalWrite(CAMERA_PIN, LOW);
      }
    }
  }

  else { // Slow speed looping
    for (int i = 0; i < NUM_LEDS; i++) {
      if (i > 0) // Turn off the last pixel
        leds[i - 1] = CRGB::Black;
      for (int j = 0; j < 3; j++) { // In fast mode totally 81 channels are shown
        if (j == 0)
          leds[i] = CRGB::Red;
        else if (j == 1)
          leds[i] = CRGB::Green;
        else
          leds[i] = CRGB::Blue;
        FastLED.show();
        digitalWrite(CAMERA_PIN, HIGH);
        CheckStopClick(pressedButtonPin);
        if (stopEarly) {
          stopEarly = false;
          break;
        }
        digitalWrite(CAMERA_PIN, LOW);
      }
    }
  }
  strncpy(currentFlashMode, flashMode.Idle, sizeof(currentFlashMode));
  UpdateDisplayInfo();
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
void UpdateDisplayInfo(bool toBlink) {
  lcd.clear();
  lcd.setCursor(1, 0);
  lcd.printstr("Delay");
  lcd.setCursor(0, 1);
  char waitTimeStr[20], tempArr[20];
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
