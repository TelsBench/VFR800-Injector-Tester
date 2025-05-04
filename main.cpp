
// @copyright telsbench MIT license
// https://youtu.be/YGfazHLdIow
// see license


#include <Arduino.h>
#include <Encoder.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Rotary Encoder pins
#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 3
#define BUTTON_PIN 4

//Buttons
#define FIRE_BUTTON_PIN 5      


//Injector Output Signal Pin
#define OUTPUT_PIN 6         


// Menu-related variables
int rpmValue = 1000;  // Default RPM value
int burstTimeValue = 1; // Default BurstTime value
int lastPosition = -1;

// RPM and BurstTime options
const int rpmOptions[] = {1000, 5000, 10000};
const int burstTimeOptions[] = {0, 1, 5};
const int frequencies[] = {11, 42, 82};             // Corresponding frequencies for RPM values
int selectedBURSTIndex = 1;
int selectedRPMIndex =0;
int settingsSelectionIndex=0;

//Create Encoder 
Encoder encoder(ENCODER_PIN_A, ENCODER_PIN_B);
bool rotaryButtonPressed = false;

//Prototypes
void DisplaySettings();
void adjustRPM() ;
void adjustBurst();
void firerotaryButtonPressed();
void SelectSetting();


void displayMessage(int x, int y, String message, int textSize = 1, bool clearScreen = false) 
{
    display.setCursor(x, y);
    display.setTextSize(textSize);
    if (clearScreen) display.clearDisplay();
    display.println(message);
    display.display();
}

void DisplaySettings()
{
  display.clearDisplay();
  displayMessage(0, 0, "RPM: " + String(rpmValue), 2);
  displayMessage(0, 18, "Burst: " + String(burstTimeValue), 2);
}

void setup() {
    pinMode(BUTTON_PIN, INPUT_PULLUP); // Button pin with pull-up
    pinMode(FIRE_BUTTON_PIN, INPUT_PULLUP); // Button pin with pull-up
    pinMode(OUTPUT_PIN, OUTPUT);
 
    Serial.begin(115200);             // Initialize serial communication
    //Start Display
    if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS))
    {
      Serial.println(F("SSD1306 allocation failed"));
      for (;;); // Stop execution if OLED initialization fails
    }

    display.setTextColor(SSD1306_WHITE);
    display.clearDisplay();
    encoder.write(0); // Reset encoder position
    display.clearDisplay();
    DisplaySettings();
}

void firerotaryButtonPressed() 
{
  Serial.println("FIRE BUTTON PIN PRESSED or LOW");
  Serial.println("FIRE BUTTON PIN PRESSED Burst Time Value " +  String(burstTimeValue));
  if (burstTimeOptions[burstTimeValue] == 0) //Manual, High as long as fire Button Pressed
  {
    // Continuous Mode: Output HIGH as long as button is pressed
    Serial.println("Continuous Mode: Output HIGH while button is pressed.");
    while (digitalRead(FIRE_BUTTON_PIN) == LOW) {
      digitalWrite(OUTPUT_PIN, HIGH);
    }
    // Ensure OUTPUT_PIN goes LOW after the button is released
    digitalWrite(OUTPUT_PIN, LOW);
    Serial.println("Button released. Output LOW.");
  } 
  else 
  {
    // Burst Modes (1 second or 5 seconds): Generate square wave for specified duration
    unsigned long duration = burstTimeValue * 1000; // Set burst time in milliseconds
    unsigned long startTime = millis();
    unsigned long endTime = startTime + duration;

    int frequency = frequencies[selectedRPMIndex]; // Frequency based on selected RPM
    float period = 1000.0 / frequency;     // Period in milliseconds
    int onTime = period * 0.6;             // 60% duty cycle ON time
    int offTime = period * 0.4;            // 40% duty cycle OFF time

    Serial.println( "frequency : " + String(frequency));
    Serial.println( "Duration : " + String(duration));
    Serial.println( "StartTime : " + String(startTime));
    Serial.println( "EndTime : " + String(endTime));

    while (millis() < endTime) 
    {
      digitalWrite(OUTPUT_PIN, HIGH);     // Square wave ON
      delay(onTime);                      // ON duration
      digitalWrite(OUTPUT_PIN, LOW);      // Square wave OFF
      delay(offTime);                     // OFF duration
    }

    // Ensure OUTPUT_PIN goes LOW after the burst duration
    digitalWrite(OUTPUT_PIN, LOW);

  }
}

void adjustRPM() 
{
  Serial.println("Entering AdjustRPM");
  while(digitalRead(BUTTON_PIN)==LOW){};//wait for Button Set RPM to release

  display.clearDisplay();
  displayMessage(0, 0, "Adjust", 2);
  displayMessage(0, 18, "RPM: " + String(rpmOptions[selectedRPMIndex]), 2); 

  while(digitalRead(BUTTON_PIN)==HIGH)
  {
    //clear part of the screen where the adjusted value will be
    displayMessage(55,0,"       ",2);
    int newPosition = encoder.read() / 4; // Adjust sensitivity
    if (newPosition != lastPosition) 
    {
      lastPosition = newPosition;
      selectedRPMIndex = abs(newPosition % 3); // Ensure valid option index
      display.clearDisplay();
      displayMessage(0, 0, "Adj RPM", 2);
      displayMessage(0, 18, "RPM: " + String(rpmOptions[selectedRPMIndex]), 2);
    }
  }

  // Save RPM and return to settings Display
  rpmValue = rpmOptions[selectedRPMIndex]; 
  display.clearDisplay();
  DisplaySettings();

  Serial.println("Return to Settings");
}

void adjustBurst() 
{
    Serial.println("Entering AdjustBurst");
    while(digitalRead(BUTTON_PIN)==LOW){};//wait for Button Set Burst  to release

    display.clearDisplay();
    displayMessage(0, 0, "Adjust", 2);
    displayMessage(0, 18, "Burst: " + String(burstTimeOptions[selectedBURSTIndex]), 2); 
    
    while(digitalRead(BUTTON_PIN)==HIGH)
    {
      //clear part of the screen where the value will be
      displayMessage(64,18,"     ",2);  
      int newPosition = encoder.read() / 4; // Adjust sensitivity
      if (newPosition != lastPosition) 
      {
         lastPosition = newPosition;
         selectedBURSTIndex = abs(newPosition % 3); // Ensure valid option index
         display.clearDisplay();
         displayMessage(0, 0, "Adjust", 2);
         displayMessage(0, 18, "Burst: " + String(burstTimeOptions[selectedBURSTIndex]), 2); // Adjusted position to (0, 18)    
      }
    }
    burstTimeValue = burstTimeOptions[selectedBURSTIndex];
    display.clearDisplay();
    DisplaySettings();
    Serial.println("BurstTime Value : " + String(burstTimeValue));
}

void SelectSetting()
{
  int settingsSelectionIndex=0;
  while( digitalRead(BUTTON_PIN)==LOW){}; //Wait for buttonup

  display.clearDisplay();
  displayMessage(0, 0, "<-Choose->", 2);

  while(digitalRead(BUTTON_PIN)==HIGH)
  {
    int newPosition = encoder.read() / 4; // Adjust sensitivity
    if (newPosition != lastPosition) 
    {
      lastPosition = newPosition;
      settingsSelectionIndex = abs(newPosition % 2); // Ensure valid option index
      display.fillRect(0,18,128,32,BLACK);
      
      settingsSelectionIndex==0 ? displayMessage(0, 18, "Burst Mode", 2) : displayMessage(0, 18, "RPM", 2);
    }
    Serial.println("Looping in SelectSetting");
  }
   
  delay(200);
  settingsSelectionIndex==0  ? adjustBurst()  :  adjustRPM();

}

void loop() 
{
   if(digitalRead(BUTTON_PIN)==LOW )  
   {
     delay(300);
     SelectSetting();
     DisplaySettings();
   } 

   if(digitalRead(FIRE_BUTTON_PIN)==LOW) firerotaryButtonPressed(); 

   Serial.println("IN MAIN LOOP");
   Serial.println("RPM : " + String(rpmValue));
   Serial.println("Burst : " + String(burstTimeValue));
   delay(300);
}
