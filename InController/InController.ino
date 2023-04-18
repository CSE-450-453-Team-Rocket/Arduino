/*
Team Rocket 
Aurthor: Marcos De La Osa Cruz
Purpose: This sketch controls the arduino located within the controller box
  It handles changes in the pressurization button, switches and joystick signals
  and sends that information to the Pi through a serial connection.
  We wanted to avoid plugging in elements that having varying amps into the Pi for hardware safety
 */ 
#include <stdbool.h>
#include <Arduino.h>
#include <ArduinoJson.h>

//-----------------PIN-I/O-----------//
#define PRESSURIZE_BUTTON 2
#define LEFT_JOY_X_PIN A0
#define LEFT_JOY_Y_PIN A1 
#define LEFT_SWITCH_PIN 3 
#define RIGHT_JOY_X_PIN A2
#define RIGHT_JOY_Y_PIN A3
#define RIGHT_SWITCH_PIN 22

#define SEND_DATA_WARNING 12      // set to high (temporarily) to cause incoming data interrupt on the Pi
#define RECIEVE_DATA_WARNING 13   // High when the pi is sending data to us (never used)

//---------------Declarations-----------//
void setUpGPIO();
void updateGPIO();
void sendResponse();

//---------------Globals-----------//
struct JoySwitchInfo {
  int _x = 0;
  int _y = 0;
  bool _sw = false;
};

bool pButton = false;
JoySwitchInfo right;
JoySwitchInfo left;

//---------------GPIO-----------//
void setUpGPIO(){
  pinMode(LEFT_JOY_X_PIN, INPUT);
  pinMode(LEFT_JOY_Y_PIN, INPUT); 
  pinMode(RIGHT_JOY_X_PIN, INPUT);
  pinMode(RIGHT_JOY_Y_PIN, INPUT);
  pinMode(LEFT_SWITCH_PIN, INPUT);
  pinMode(RIGHT_SWITCH_PIN, INPUT);
  pinMode(PRESSURIZE_BUTTON, INPUT);
  return;
}

// Save the most current readings to structures
void updateGPIO(){
    right._x = analogRead(RIGHT_JOY_X_PIN);
    right._y = analogRead(RIGHT_JOY_Y_PIN);
    right._sw = digitalRead(RIGHT_SWITCH_PIN);
    left._x = analogRead(LEFT_JOY_X_PIN);
    left._y = analogRead(LEFT_JOY_Y_PIN);
    left._sw = digitalRead(LEFT_SWITCH_PIN);
    return;
}

//---------------Serial-----------//
void serialSetUp(){
  pinMode(SEND_DATA_WARNING, OUTPUT);
  pinMode(RECIEVE_DATA_WARNING, INPUT);
  Serial.begin(9600);     //Debugging serial Monitor
}

// send a response packet to the Pi
void sendResponse() {

  StaticJsonDocument<192> doc;    //Create response packet
  doc["_COMMENT"] = "json format is sent from the arduino in the controller to the Pi";

  JsonObject Joysticks = doc.createNestedObject("Joysticks");

  JsonObject Joysticks_left = Joysticks.createNestedObject("left");
  Joysticks_left["x"] = left._x;
  Joysticks_left["y"] = left._y;

  JsonObject Joysticks_right = Joysticks.createNestedObject("right");
  Joysticks_right["x"] = right._x;
  Joysticks_right["y"] = right._y;

  JsonObject Switches = doc.createNestedObject("Switches");
  Switches["leftOn"] = left._sw;
  Switches["rightOn"] = right._sw;
  doc["Button"]["Pressed"] = pButton;

  size_t bytesWritten = serializeJson(doc, Serial);
  Serial.write('\n');

  // Lets the Pi know wesent a message (Pi interrupts triggered)
  digitalWrite(SEND_DATA_WARNING, HIGH);
  delay(100);
  digitalWrite(SEND_DATA_WARNING, LOW);
  return;
}


//---------------Givens-----------//
void setup() {
  serialSetUp();          // Set up serial communication
  setUpGPIO();       //Set up joystick pins
}

void loop() {
  if (digitalRead(PRESSURIZE_BUTTON), HIGH) {
    pButton = true;
  }
  updateGPIO();
  delay(10);
  sendResponse();

  delay(500);
}
