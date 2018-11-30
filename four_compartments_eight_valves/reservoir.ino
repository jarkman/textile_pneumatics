
Reservoir::Reservoir(int p,  int s)
{
  pumpPin = p;

  sensorPin = s;

  targetPressure = 50;
  deadband = 5;
  smoothedPressure = 0.0;
  pumpDelta = 0.004; // pump speed change per loop
  
  pinMode(pumpPin, OUTPUT);
  //setPwmFrequency(pumpPin, 1024 ); // 31khz for whine reduction
  digitalWrite(pumpPin,0);
}

void Reservoir::loop()
{
  readPressure();

  if( isIdle() )
  {
    digitalWrite(pumpPin,0);
    return;
  }
  
  if( doPwmPumpControl )
  {
    setPumpSpeed( pumpSpeed +  (targetPressure-pressure) / 1000.0); // speed dependent on how far we have to go

  }
  else
  {
    if( digitalRead( pumpPin ))
    {
      if(  pressure > targetPressure  ) // reached target, turn off
        digitalWrite( pumpPin, false );
    }
    else
    {
      if(  pressure < targetPressure - deadband ) // below deadband, turn on
        digitalWrite( pumpPin, true );
    }
  }

    
 
}

void Reservoir::setPumpSpeed(float p)
{

  if( p > 1.0 )
    p = 1.0;
  if( p < 0.0 )
    p = 0.0;

      
  //if( p > 0.0 )
  //{
  //  p = fconstrain(p, pumpSpeed-pumpDelta, pumpSpeed+pumpDelta );
    
  //}

  //p = 1.0;
  
  pumpSpeed = p;

  if( p > 0.1 )

    p = p * 0.2 + 0.8; // pump starts spinning at 0.3

  else
    p = 0;
    
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

// https://playground.arduino.cc/Code/PwmFrequency
void setPwmFrequency(int pin, int divisor) {
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if(pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if(pin == 3 || pin == 11) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x07; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}

