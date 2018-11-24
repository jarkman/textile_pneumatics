
#include <Wire.h> 

#include "chamber.h"
#include "reservoir.h"

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

boolean traceSupermanual = false;
float supermanual[]={0,0,0,0};
boolean gotSupermanual = false;

long startTime;
int mode = 0;
long modeStart = 0;


long bendPeriod = 30000;
long strengthPeriod = 10L*10000L;

long dur = 10000L;
float f = 0.0;


void setup1msTimer()
{

  // Timer0 is already used for millis() - we'll just interrupt somewhere
  // in the middle 
  OCR0A = 0xAF;
  TIMSK0 |= _BV(OCIE0A);
} 
 
// Interrupt is called once a millisecond
SIGNAL(TIMER0_COMPA_vect) 
{

  chamber1.oneMsTimer();
  chamber2.oneMsTimer();
  chamber3.oneMsTimer();
  chamber4.oneMsTimer();
}

void setupI2C()
{
  Wire.begin();
  Wire.setClock(10000);  // 10k for a 10m wire length limit - esp seems to ignore this! Currently getting a 53khz clock

}


void setup() {
  Serial.begin(9600);

  setup1msTimer();
  Serial.println("");
  Serial.println("---Setup---");
  Serial.println("..i2c");
  setupI2C();

    Serial.println("..supermanual");
  setupSupermanual();
  
  chamber1.setTargetPressure(0);
  chamber2.setTargetPressure(0);
  chamber3.setTargetPressure(0);
  chamber4.setTargetPressure(0);
  startTime = millis();
  mode = 0;
  modeStart = millis();
}


float fmap(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}



void loop() {
  loopSupermanual();
  reservoir.loop();
  baselinePressure = reservoir.targetPressure - 2.0;
  chamber1.loop();
  chamber2.loop();
  chamber3.loop();
  chamber4.loop();
  if( loopSupermanualControl())
    return;
    
  loopCatEar();
  //loopDoubleFrondEar();

  printGraph();

  printPressures("Ear: ");
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

  printPressures("Supermanual: ");

  return true;
}

void loopDoubleFrondEar()
{
  
  long now = millis();

  if( millis() - modeStart > dur)
  {
    mode++;
    if( mode > 3 )
      mode = 0;
    modeStart = millis();
    
  }

  f = (float) (millis()-modeStart)/(float) dur;
  
  if( false )
  {
    chamber1.setTargetFraction( 0.0);
      chamber2.setTargetFraction( 1.0);
      chamber3.setTargetFraction( 0.0  );
  }
  else
  {
  switch( mode )
  {
    case 0:
    dur = 4000;
      chamber1.setTargetFraction( f);
      chamber2.setTargetFraction( 0.0);
      chamber3.setTargetFraction( 0 );
      break;

    case 1:
      chamber1.setTargetFraction( 1.0-f);
      chamber2.setTargetFraction( 0.0);
      chamber3.setTargetFraction( 0 );
      break;
      
   case 2:
      chamber1.setTargetFraction( 0);
      chamber2.setTargetFraction( f);
      chamber3.setTargetFraction(0);
      //dur = 2000;
      break;


    case 3:
      chamber1.setTargetFraction( 0);
      chamber2.setTargetFraction( 1.0-f);
      chamber3.setTargetFraction( 0 );
      break;
      

     
    case 4:
    dur = 2000;
      chamber1.setTargetFraction( 0.0);
      chamber2.setTargetFraction( 0.0);
      chamber3.setTargetFraction( 0.0  );
      break;
      

      
  }
  }

  

  }
void loopCatEar() {

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
    if( mode > 3 )
      mode = 0;
    modeStart = millis();
    
  }

  f = (float) (millis()-modeStart)/(float) dur;

    dur = 2000;
    
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
    //dur = 1500;
      chamber1.setTargetFraction( f);
      chamber2.setTargetFraction( 0.0);
      chamber3.setTargetFraction( f );
      break;

    case 1:
      chamber1.setTargetFraction( 1.0-f);
      chamber2.setTargetFraction( f );
      chamber3.setTargetFraction( 1.0-f );
      break;
      
   case 2:
      chamber1.setTargetFraction( f);
      chamber2.setTargetFraction( 1.0-f);
      chamber3.setTargetFraction( f );
      //dur = 1200;
      break;


    case 3:
      chamber1.setTargetFraction( 1.0-f);
      chamber2.setTargetFraction( 1.0-f);
      chamber3.setTargetFraction( 1.0-f );
      break;
      

     
    case 4:
    //dur = 2000;
      chamber1.setTargetFraction( 0.0);
      chamber2.setTargetFraction( 0.0);
      chamber3.setTargetFraction( 0.0  );
      break;
      

      
  }
  }
   chamber1.loop();
 chamber2.loop();
 chamber3.loop();


}

void printGraph()
{

    Serial.print(reservoir.pressure);   
    Serial.print(",");

    Serial.print(chamber1.targetPressure);
    Serial.print(",");
    Serial.print(chamber1.pressure);
    Serial.print(",");
    Serial.print(chamber1.state);
  
    Serial.println();
}



long lastPrintPressures = 0;
void printPressures(char*label)
{

  return;
  
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

