#include <QC2Control.h>


#include <Wire.h> 

#include "chamber.h"
#include "reservoir.h"

#define LED_PIN 13 

//Pin 4 for Data+
//Pin 5 for Data-
//See How to connect in the documentation for more details.
// https://github.com/septillion-git/QC2Control
QC2Control quickCharge(11,12);


const int reservoirPumpPin = 10; // must be a PWM pin - 3, 5, 6, 9, 10, and 11 on Nano

const int inflate1Pin = 3; 
const int inflate2Pin = 5; 
const int inflate3Pin = 7; 
const int inflate4Pin = 9; 

const int deflate1Pin = 2;
const int deflate2Pin = 4;
const int deflate3Pin = 6;
const int deflate4Pin = 8;

const int pressureReservoirPin = 0;

const int pressure1Pin = 1;
const int pressure2Pin = 2;
const int pressure3Pin = 3;
const int pressure4Pin = 6;


Chamber chamber1(inflate1Pin, deflate1Pin, pressure1Pin);
Chamber chamber2(inflate2Pin, deflate2Pin, pressure2Pin);
Chamber chamber3(inflate3Pin, deflate3Pin, pressure3Pin);
Chamber chamber4(inflate4Pin, deflate4Pin, pressure4Pin);

Reservoir reservoir(reservoirPumpPin, pressureReservoirPin);

float baselinePressure = 0.0; 

boolean trace = false;
boolean tracePressures = false;
boolean traceGraph = true;

boolean doPwmPumpControl = false;

boolean traceSupermanual = false;
float supermanual[]={0,0,0,0};
boolean gotSupermanual = false;
boolean gotImu = false;


float imuNod = 0.0;   // nod amount from -1.0 to 1.0
float imuTurn = 0.0; // head turn amount (rightwards) from -1.0 to 1.0
float imuLean = 0.0; // head lean amount leftwards

float lCurl;
float rCurl;
float lStraighten;
float rStraighten;
  
long startTime;
int mode = 0;
long modeStart = 0;


long bendPeriod = 30000;
long strengthPeriod = 10L*10000L;

long dur = 10000L;

long loopDuration = 4; // tracks measured loop duration



bool blinkState = false;


void setup1msTimer()
{

  // Timer0 is already used for millis() - we'll just interrupt somewhere
  // in the middle 
  OCR0A = 0x20;
  OCR0B = 0x9F;
  TIMSK0 |= _BV(OCIE0A);
  TIMSK0 |= _BV(OCIE0B);
} 
 
// Interrupt is called once a millisecond
SIGNAL(TIMER0_COMPA_vect) 
{

  chamber1.oneMsTimer();
  chamber2.oneMsTimer();
  chamber3.oneMsTimer();
  chamber4.oneMsTimer();
}

SIGNAL(TIMER0_COMPB_vect) 
{

  chamber1.oneMsTimer();
  chamber2.oneMsTimer();
  chamber3.oneMsTimer();
  chamber4.oneMsTimer();
}

void setupI2C()
{
  Wire.begin();
  //Wire.setClock(10000);  // 10k for a 10m wire length limit - esp seems to ignore this! Currently getting a 53khz clock

}


void setup() {
  Serial.begin(115200);

  
  Serial.println("");
  Serial.println("---Setup---");
  
  
  if( trace ) Serial.println("setupI2C");
  setupI2C();
  if( trace ) Serial.println("setupMpu6050");
  setupMpu6050();
  if( trace ) Serial.println("1ms timer");
  setup1msTimer();
  

  if( trace ) Serial.println("setupSupermanual");
  setupSupermanual();
  
  chamber1.setTargetPressure(0);
  chamber2.setTargetPressure(0);
  chamber3.setTargetPressure(0);
  chamber4.setTargetPressure(0);
  startTime = millis();
  mode = 0;
  modeStart = millis();

  if( trace ) Serial.println("quickCharge.set12V");
  //set voltage to 12V
  quickCharge.set12V();
  
  if( trace ) Serial.println("setup done");
}


float fmap(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}



void loop() {
  long loopStart = millis();
  
  timedLoop();
  
  if( trace ) Serial.println("blink");
    // blink LED to indicate activity
    blinkState = !blinkState;
    digitalWrite(LED_PIN, blinkState);
    
  if( tracePressures ) printPressures("  ");
  if( traceGraph) printGraph();
  
  long loopEnd = millis();
  loopDuration = loopEnd-loopStart;
  //Serial.print("loop took ");
  //Serial.println(loopDuration);
}

void timedLoop()
{

  if( trace ) Serial.println("loopMpu6050");
  loopMpu6050();
  if( trace ) Serial.println("loopSupermanual");
  loopSupermanual();
  if( trace ) Serial.println("reservoir.loop");
  reservoir.loop();
  baselinePressure = reservoir.smoothedPressure - 2.0;
  if( trace ) Serial.println("chamber1.loop");
  chamber1.loop();
  chamber2.loop();
  chamber3.loop();
  chamber4.loop();
  
  if( trace ) Serial.println("loopSupermanualControl");
  if( loopSupermanualControl())
    return;



  if( trace ) Serial.println("loopImuPose");
  if( loopImuPose())
    return;


  if( trace ) Serial.println("loopCatEar");
  loopCatEar();
  //loopDoubleFrondEar();

 

 
}

boolean loopSupermanualControl()
{

  if( ! gotSupermanual /*|| supermanualIdle()*/)
  {
    return false;
  }

   
  chamber1.setTargetPressure(baselinePressure * supermanual[0]);
  chamber2.setTargetPressure(baselinePressure * supermanual[1]);
  chamber3.setTargetPressure(baselinePressure * supermanual[2]);
  chamber4.setTargetPressure(baselinePressure * supermanual[3]);

  

  return true;
}

boolean loopImuPose()
{
  if( ! gotImu )
    return false;

  // linear mixing of our two behaviours
  float nodFraction = fabs( imuNod )/(fabs( imuNod ) + fabs( imuTurn ) + 0.1);
  float turnFraction = fabs( imuTurn )/(fabs( imuNod ) + fabs( imuTurn ) + 0.1);

  lCurl = 0;
  rCurl = 0;
  lStraighten = 0;
  rStraighten = 0;
  

  // nod bends us forwards
    lCurl += nodFraction * imuNod;
    rCurl += nodFraction * imuNod;
    

  // turn does one side not the other
  // turn has extra curl
   double turnExtra = 0.3;
   
    lCurl += turnFraction * (turnExtra + imuTurn);
    rCurl += turnFraction * (turnExtra - imuTurn);

    // straighten is always inverse of curl
    lStraighten = ( 1.0-lCurl );
    rStraighten = ( 1.0-rCurl );

    

  
  lCurl = fconstrain(lCurl, 0.0, 1.0);
  rCurl = fconstrain(rCurl, 0.0, 1.0);
  lStraighten = fconstrain(lStraighten, 0.0, 1.0);
  rStraighten = fconstrain(rStraighten, 0.0, 1.0);
  
  chamber1.setTargetFraction( lCurl);
  chamber2.setTargetFraction( lStraighten);
  chamber3.setTargetFraction( rStraighten);
  chamber4.setTargetFraction( rCurl);

  return true;
}


void loopCatEar() {


 dur = 2000;

 
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

  if( millis() - modeStart > dur)
  {
    mode++;
    if( mode > 1 )
      mode = 0;
    modeStart = millis();
    
  }

  float f = (float) (millis()-modeStart)/(float) dur;

   
    
  if( false )
  {
    chamber1.setTargetFraction( 0.0);
      chamber2.setTargetFraction( 1.0);
      chamber3.setTargetFraction( 1.0  );
  }
  else
  {
    switch( mode )
    {
      case 0:
     
        chamber1.setTargetFraction( f);
        chamber2.setTargetFraction( f);
        chamber3.setTargetFraction( 1.0-f );
        chamber4.setTargetFraction( 1.0-f );
        break;
  
      case 1:
        chamber1.setTargetFraction( 1.0-f);
        chamber2.setTargetFraction( 1.0-f);
        chamber3.setTargetFraction( f );
        chamber4.setTargetFraction( f );
        break;
   
      }
  }
   
}

long lastPrintGraph = 0;

void printGraph()
{

    if( millis() - lastPrintGraph < 20 )
      return;

    lastPrintGraph = millis();
    /*
    Serial.print(10.0 * reservoir.pumpSpeed);   
    Serial.print(",");
    Serial.print(reservoir.targetPressure);   
    Serial.print(",");
   */
   /*
    Serial.print(reservoir.pressure);   
    Serial.print(",");


    Serial.print(chamber1.targetPressure);
    Serial.print(",");
    Serial.print(chamber1.pressure);
    Serial.print(",");
    Serial.print(chamber1.state);
    Serial.print(",");
    */
    /*
    Serial.print(10.0*imuNod);
    Serial.print(",");
    Serial.print(10.0*imuTurn);
  */

    Serial.print(lCurl);
    Serial.print(","); 
    Serial.print(lStraighten);
    Serial.print(","); 
    Serial.print(rStraighten);
    Serial.print(",");
    Serial.print(rCurl);
    Serial.print(","); 
    
    
  
  
    Serial.println();
}



long lastPrintPressures = 0;
void printPressures(char*label)
{

  if( millis() - lastPrintPressures < 1000 )
    return;

  lastPrintPressures = millis();
    

    Serial.print(label);

    Serial.print(" Reservoir ");
    Serial.print("T:");
    Serial.print(reservoir.targetPressure);
    Serial.print(" A:");
    Serial.print(reservoir.pressure);
    
    Serial.print(" | 1 ");
    Serial.print("T:");
    Serial.print(chamber1.targetPressure);
    Serial.print(" A:");
    Serial.print(chamber1.pressure);

        Serial.print(" | 2 ");
    Serial.print("T:");
    Serial.print(chamber2.targetPressure);
    Serial.print(" A:");
    Serial.print(chamber2.pressure);

    Serial.print(" | 3 ");
    Serial.print("T:");
    Serial.print(chamber3.targetPressure);
    Serial.print(" A:");
    Serial.print(chamber3.pressure);

    Serial.print(" | 4 ");
    Serial.print("T:");
    Serial.print(chamber4.targetPressure);
    Serial.print(" A:");
    Serial.print(chamber4.pressure);

  Serial.println(" ");


}

float fconstrain(float f, float out_min, float out_max)
{
  if( f < out_min )
    f = out_min;

  if( f > out_max )
    f = out_max;

  return f;
}

