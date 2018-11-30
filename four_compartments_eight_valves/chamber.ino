
Chamber::Chamber(int i, int d, int s)
{
  inflatePin = i;
  deflatePin = d;
  sensorPin = s;

  deadband = 0.25; // in kpa
  pulseFactor = 1.0; // gives us an effective deadband, since pulse times are quantised to 1ms
  
  targetPressure = 0;
  
  pinMode(inflatePin, OUTPUT);
  pinMode(deflatePin, OUTPUT);

  digitalWrite(inflatePin,0);
  digitalWrite(deflatePin,0);
}

void Chamber::loop()
{
  readPressure();

  if( isIdle() )
  {
    digitalWrite(inflatePin,0);
    digitalWrite(deflatePin,0);
    state = 0;
    return;
  }

  float p = pressure;
  
  if( fabs( targetPressure - p ) < deadband/2.0 )
  {
    digitalWrite(inflatePin,0);
    digitalWrite(deflatePin,0);
    state = 0;
    return;
  }

  remainingMs = pulseFactor*fabs(p-targetPressure);
  if( remainingMs < 1 )
  {
    // too short a pulse to do
    remainingMs = 0;
    state = 0;
    return;
  }

  if( remainingMs > loopDuration )
    remainingMs = loopDuration;
    
  if( targetPressure < p )
  {
    digitalWrite(inflatePin,0);
    digitalWrite(deflatePin,1); // let some air out
    
    state = -1;
  }
  else
  {
    digitalWrite(inflatePin,1);
    digitalWrite(deflatePin,0); 
    
    
    state = 1;
  }
}

void Chamber::oneMsTimer()
{
  if(remainingMs < 0 )
    return;
    
  remainingMs--;
  if(remainingMs < 0 )
  {
    digitalWrite(inflatePin,0);
    digitalWrite(deflatePin,0);
    state=0;
  }
}

void Chamber::setTargetFraction(float f)
{
  targetPressure = f * baselinePressure;
}
void Chamber::setTargetPressure( float t )
{
  targetPressure = t;
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

  smoothedPressure = 0.9 * smoothedPressure + 0.1 * pressure;

  
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

