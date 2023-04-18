/*
Team Rocket 
Aurthor: Marcos De La Osa Cruz
Purpose: This sketch tests the pressurization sequence upon start up
 */ 

#include <stdbool.h>
#include <Arduino.h>
#include <ArduinoJson.h>

//---------------PIN-I/O-----------//
#define ACTUATOR_1_POS_PIN A13
#define ACTUATOR_1_L_ALARM_PIN A14
#define ACTUATOR_1_R_ALARM_PIN A15

#define ACTUATOR_1_EXTEND_PIN 2
#define ACTUATOR_1_RETRACT_PIN 3

#define ACTUATOR_MAX_SPEED 255
#define ACTUATOR_MIN_SPEED 5
#define ACTUATOR_SLOW_DISTANCE 150
#define ACTUATOR_PRECISION 1

// pneumatics - not wired officially yet
#define SOLENOID_A 12
#define SOLENOID_B 13
#define SOLENOID_C 14
#define AIR_COMPRESSOR 15
#define RIGHT_LATCH 16
#define LEFT_LATCH 17 
#define PRESSURE_SENSOR A6

// pressurization signals (digtial)
#define STAGE_1 22
#define STAGE_2 24
#define STAGE_3 26 
#define STAGE_4 28
#define STAGE_5 30

// Serial communication
#define SEND_DATA_WARNING 33      // set to high (temporarily) to cause incoming data interrupt on the Pi
#define RECIEVE_DATA_WARNING 34   // High when the pi is sending data to us (never used)



//---------------Declarations-----------//
void pressurize(int waitTime, int presTime, int pres); // When true that side will be pressurized

typedef struct activeInfo{
  bool left;
  bool right;
};

activeInfo active;    // holds the activity state sent from the pi

//---------------PRESSURIZATION-----------//
 /******************************************\
 Pressurization command stages

 1.) Close the latches to prevent rockets from escaping during pressurization

 2.) Close the opposite valves of the rockets that are being fired, to prevent 
 air flowing through the valves that don't contain rockets

 3.) Turn on the air compressor for designated amo;unt of time to allow system
 to pressurize the rockets. In the case the system doesn't pressurize in this amount
 of time a safety shut off will occur

 4.) Turn off the air compressor and measure the system pressure for a set interval
 This checks for leaks in the rocket design, and prevents faulty pressurized launches

 \******************************************/
#define WAITTIME 5000 // ms before stage 3 time out
#define PRESREAD 100  // ms between pressure sensor reads
#define PRESTIME 2000 // ms before stage 4 pass
#define PRESSURE 60   // psi to pressurize & hold
#define H_VALUE  12   // hysterisa of accepted similarity

// Creates ban of accepted values
bool hysterisa(int desired, int actual){
 return (actual + H_VALUE < desired || actual - H_VALUE > desired) ? true : false;
}

// Handles error in pressurization sequence - doesn't do much now lol
void perror(int stage_number){
  switch (stage_number){
    case (1):
      break;
    case (2):
      break;
    case (3):
      break;
    case (4):
      break;
    case (5):
      break;
    default:
      break;
  }
  return;
}

void pressurize(int waitTime, int presTime, int pres){

  // Stage 1 - closing latches and emergency release
  digitalWrite(SOLENOID_A, false);
  digitalWrite(LEFT_LATCH, true);
  digitalWrite(RIGHT_LATCH, true);
  digitalWrite(STAGE_1, true);
  delay(100);

  // Stage 2 - closing appropriate valves
  if (active.left){
    digitalWrite(SOLENOID_B, true);
  }
  if (active.right){
    digitalWrite(SOLENOID_C, true);
  }
  digitalWrite(STAGE_2, true); 
  delay(100);

  // Stage 3 - turning on air compressor & continuously reading pressure 
  int timeElapsed = 0;
  int curpres = readPressureSensor();
  while (WAITTIME - timeElapsed > 0){
    if (timeElapsed % PRESREAD == 0){
      curpres = readPressureSensor();
      if (curpres > PRESSURE){
        break;
      }
    }
    delay(1);
    timeElapsed++;
  }
  if (timeElapsed > WAITTIME){ //took too long to pressurize
    perror(3);
  }
  digitalWrite(STAGE_3, true);
  delay(100);

  // Stage 4 - Turn off air compressor & hold desired pressure for set time interval
  digitalWrite(AIR_COMPRESSOR, false);
  timeElapsed = 0;
  while (PRESTIME - timeElapsed > 0){
    if (timeElapsed % PRESREAD == 0){
      curpres = readPressureSensor();
      if (!hysterisa(PRESSURE, curpres)){
        break;
      }
    }
    delay(1);
    timeElapsed++;
  }
  if (timeElapsed < PRESTIME){ //took too long to pressurize
    perror(4);
  }

  // Stage 5 - Lock air into rocket drums, and depressurize for back flow

  return;
}

//---------------PNEUMATICS-----------//
void setUpPneumatics(){
  pinMode(SOLENOID_A, OUTPUT);
  pinMode(SOLENOID_B, OUTPUT);
  pinMode(SOLENOID_C, OUTPUT);
  pinMode(RIGHT_LATCH, OUTPUT);
  pinMode(LEFT_LATCH, OUTPUT);  
  pinMode(AIR_COMPRESSOR, OUTPUT);
  return;
}

void setUpStageLEDs(){
  pinMode(STAGE_1, OUTPUT);
  pinMode(STAGE_2, OUTPUT);
  pinMode(STAGE_3, OUTPUT);
  pinMode(STAGE_4, OUTPUT);
  pinMode(STAGE_5, OUTPUT);
  return;
}

// Creates flowing graphics
void stageLEDFlow(){
    for (int i = 0; i < 3; i++){
        digitalWrite(STAGE_1, true);
        delay(100);
        digitalWrite(STAGE_2, true);
        delay(100);
        digitalWrite(STAGE_3, true);
        delay(100);
        digitalWrite(STAGE_4, true);
        delay(100);
        digitalWrite(STAGE_5, true);
        delay(100);
        digitalWrite(STAGE_1, false);
        delay(100);
        digitalWrite(STAGE_2, false);
        delay(100);
        digitalWrite(STAGE_3, false);
        delay(100);
        digitalWrite(STAGE_4, false);
        delay(100);
        digitalWrite(STAGE_5, false);
        delay(100);
    }
    return;
}

void setup() {
    setUpPneumatics();
    setUpStageLEDs();
    stageLEDFlow();
    Serial.begin(9600);
    act.right = true;
    ac.left = true;
    pressurization(WAITTIME, PRESTIME, PRESSURE);
}

void loop() {
    delay(100);
}

