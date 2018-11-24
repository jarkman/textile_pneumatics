
Reservoir::Reservoir(int p,  int s)
{
  pumpPin = p;

  sensorPin = s;

  targetPressure = 25;
  deadband = 2;
  smoothedPressure = 0.0;
  
  pinMode(pumpPin, OUTPUT);
  digitalWrite(pumpPin,0);
}

void Reservoir::loop()
{
  readPressure();

  if(  pressure > targetPressure  )
    pumpSpeed(0.0);
  else
    pumpSpeed( 0.9 ); // (targetPressure-pressure) / 0.5 ); // speed dependent on how far we have to go

 
}

void Reservoir::pumpSpeed(float p)
{
  if( p > 1.0 )
    p = 1.0;
  if( p < 0.0 )
    p = 0.0;
      
  if( p > 0.0 )
    p = p * 0.8 + 0.2; // pump starts spinning at 0.3
    
  analogWrite(pumpPin, 255.0* p);

  //Serial.print("                                      ");
 //Serial.print(p);
 
 //Serial.println(" kpa" );
}

void Reservoir::setTargetPressure( float t )
{
  targetPressure = t;
}



void Reservoir::readPressure()
{
  int a = analogRead(sensorPin);
  float v = 5.0 * (float) a / 1024.0;

  // we are using ABPMANN004BGAA5, https://uk.rs-online.com/web/p/differential-pressure-sensor-ics/9220925/?sra=pstk
  // a 4 bar, 5 volt device
  // see http://docs-europe.electrocomponents.com/webdocs/14b9/0900766b814b9b6f.pdf

  // Voltage goes from 0.5V to 4.5V over 0 to 4 bar

  float kpa = fmap(v, 0.5, 4.5, 0.0, 4.0 * 100.0 ); // in kpa
  pressure = 0.1*kpa+0.9*pressure;

  smoothedPressure = 0.99 * smoothedPressure + 0.01 * pressure;

  /*
  Serial.print("pin " );
  Serial.print(sensorPin);
  Serial.print(" " );
  Serial.print(v);
  Serial.print(" V " );
  Serial.print(kpa);
  Serial.println(" kpa" );
*/
 

}

