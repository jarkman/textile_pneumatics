
#include "chamber.h"

const int pump1Pin = 3; // must be a PWM pin
const int pump2Pin = 5; // must be a PWM pin
const int valve1Pin = 6;
const int valve2Pin = 7;

const int pressure1Pin = 0;
const int pressure2Pin = 1;


Chamber chamber1(pump1Pin, valve1Pin, pressure1Pin);
Chamber chamber2(pump2Pin, valve2Pin, pressure2Pin);

long startTime;

void setup() {
  Serial.begin(9600);

  chamber1.setTargetPressure(0);
  chamber2.setTargetPressure(0);
  startTime = millis();
}


float fmap(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

long bendPeriod = 15000;
long strengthPeriod = 10L*10000L;


void loop() {

 long now = millis();
 long bendPhaseMillis = (now - startTime) % bendPeriod;
 float bendPhase = 2.0 * 3.14 * (float)bendPhaseMillis / (float) bendPeriod;
 float bendPosition = (1.0+sin(bendPhase))/2.0;

 long strengthPhaseMillis = (now - startTime) % strengthPeriod;
 float strengthPhase = 2.0 * 3.14 * (float)strengthPhaseMillis / (float) strengthPeriod;
 float strengthPosition = (1.0+sin(strengthPhase))/2.0;

strengthPosition = 1.0;

 Serial.print(strengthPosition);
 Serial.print(" ");
 
 Serial.print(bendPosition);
 Serial.print(" ");
 chamber1.setTargetFraction( strengthPosition * bendPosition);
 chamber2.setTargetFraction( strengthPosition * (1.0-bendPosition));
 
 chamber1.loop();
 chamber2.loop();

 Serial.print(chamber1.targetPressure);
 Serial.print(" - ");
 Serial.print(chamber1.pressure);

 Serial.print("       ");
 
 Serial.print(chamber2.targetPressure);
 Serial.print(" - ");
 Serial.print(chamber2.pressure);
 
 Serial.println(" kpa" );

  
}


