
Chamber::Chamber(int p, int v, int s)
{
  pumpPin = p;
  valvePin = v;
  sensorPin = s;

  deadband = 4; //2; // in kpa
  maxPressure = 60; //45; // limit of our pumps
  
  targetPressure = 0;
  
  pinMode(pumpPin, OUTPUT);
  pinMode(valvePin, OUTPUT);

  pumpSpeed( 0.0);
 
  digitalWrite(valvePin,0);
}

void Chamber::loop()
{
  readPressure();
  
  if( fabs( targetPressure - pressure ) < deadband/2.0 )
  {
    pumpSpeed( 0.0);
 
    digitalWrite(valvePin,0);
    return;
  }
  
  if( targetPressure < pressure )
  {
    pumpSpeed( 0.0);
 
    digitalWrite(valvePin,1); // let some air out
  }
  else
  {
    pumpSpeed( (targetPressure-pressure) / 8.0 ); // speed dependent on how far we have to go
 
    digitalWrite(valvePin,0); 
  }
}

void Chamber::setTargetFraction(float f)
{
  targetPressure = f * maxPressure;
}
void Chamber::setTargetPressure( float t )
{
  targetPressure = t;
}

void Chamber::pumpSpeed(float p)
{
  if( p > 1.0 )
    p = 1.0;
  if( p < 0.0 )
    p = 0.0;
      
  if( p > 0.0 )
    p = p * 0.7 + 0.3; // pump starts spinning at 0.3
  analogWrite(pumpPin, 255.0* p);

  //Serial.print("                                      ");
 //Serial.print(p);
 
 //Serial.println(" kpa" );
}

void Chamber::readPressure()
{
  int a = analogRead(sensorPin);
  float v = 5.0 * (float) a / 1024.0;

  // we are using ABPMANN004BGAA5, https://uk.rs-online.com/web/p/differential-pressure-sensor-ics/9220925/?sra=pstk
  // a 4 bar, 5 volt device
  // see http://docs-europe.electrocomponents.com/webdocs/14b9/0900766b814b9b6f.pdf

  // Voltage goes from 0.5V to 4.5V over 0 to 4 bar

  float kpa = fmap(v, 0.5, 4.5, 0.0, 4.0 * 100.0 ); // in kpa
  pressure = 0.1*kpa+0.9*pressure;
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

