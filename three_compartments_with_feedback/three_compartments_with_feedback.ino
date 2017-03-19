
#include "chamber.h"

const int pump1Pin = 3; // must be a PWM pin - 3, 5, 6, 9, 10, and 11 on Nano
const int pump2Pin = 5; // must be a PWM pin
const int pump3Pin = 6; // must be a PWM pin

const int valve1Pin = 7;
const int valve2Pin = 8;
const int valve3Pin = 12;

const int pressure1Pin = 0;
const int pressure2Pin = 1;
const int pressure3Pin = 2;


Chamber chamber1(pump1Pin, valve1Pin, pressure1Pin);
Chamber chamber2(pump2Pin, valve2Pin, pressure2Pin);
Chamber chamber3(pump3Pin, valve3Pin, pressure3Pin);

long startTime;
int mode = 0;
long modeStart = 0;

void setup() {
  Serial.begin(9600);

  chamber1.setTargetPressure(0);
  chamber2.setTargetPressure(0);
  chamber3.setTargetPressure(0);
  startTime = millis();
  mode = 0;
  modeStart = millis();
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

/*
 Serial.print(strengthPosition);
 Serial.print(" ");
 
 Serial.print(bendPosition);
 Serial.print(" ");
 chamber1.setTargetFraction( strengthPosition * bendPosition);
 chamber2.setTargetFraction( strengthPosition * (1.0-bendPosition));
 chamber3.setTargetFraction( 1.0 - strengthPosition );
 

 Serial.print(chamber1.targetPressure);
 Serial.print(" - ");
 Serial.print(chamber1.pressure);

 Serial.print("       ");
 
 Serial.print(chamber2.targetPressure);
 Serial.print(" - ");
 Serial.print(chamber2.pressure);
 
 Serial.println(" kpa" );
*/

  if( millis() - modeStart > 5000L)
  {
    mode++;
    if( mode > 3 )
      mode = 0;
    modeStart = millis();
  }
  
  switch( mode )
  {
    case 0:
      chamber1.setTargetFraction( 0.0);
      chamber2.setTargetFraction( 0.0);
      chamber3.setTargetFraction( 1.0  );
      break;

    case 1:
      chamber1.setTargetFraction( 1.0);
      chamber2.setTargetFraction( 0.0);
      chamber3.setTargetFraction( 1.0  );
      break;
      
   case 2:
      chamber1.setTargetFraction( 0.0);
      chamber2.setTargetFraction( 0.0);
      chamber3.setTargetFraction( 1.0  );
      break;


    case 3:
      chamber1.setTargetFraction( 0.0);
      chamber2.setTargetFraction( 1.0);
      chamber3.setTargetFraction( 1.0  );
      break;
      

     
      
  }

   chamber1.loop();
 chamber2.loop();
 chamber3.loop();

}


