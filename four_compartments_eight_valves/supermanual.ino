#include <Adafruit_ADS1015.h>

// runs a ADS1015 on multiplexer output 0, with 3 sliders
// sliders control servo drive directly
 
// Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */
Adafruit_ADS1015 ads;     /* Use thi for the 12-bit version */



long lastSupermanualTime = 0;
float minAdc = 0.0;
float maxAdc = 2050.0;

void setupSupermanual(void) 
{
 
  
  //Serial.println("Getting single-ended readings from AIN0..3");
  //Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)");
  
  // The ADC input range (or gain) can be changed via the following
  // functions, but be careful never to exceed VDD +0.3V max, or to
  // exceed the upper and lower limits if you adjust the input range!
  // Setting these values incorrectly may destroy your ADC!
  //                                                                ADS1015  ADS1115
  //                                                                -------  -------
  // ads.setGain(GAIN_TWOTHIRDS);  // 2/3x gain +/- 6.144V  1 bit = 3mV      0.1875mV (default)
  // ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV
  // ads.setGain(GAIN_TWO);        // 2x gain   +/- 2.048V  1 bit = 1mV      0.0625mV
  // ads.setGain(GAIN_FOUR);       // 4x gain   +/- 1.024V  1 bit = 0.5mV    0.03125mV
  // ads.setGain(GAIN_EIGHT);      // 8x gain   +/- 0.512V  1 bit = 0.25mV   0.015625mV
  // ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV

 
  lastSupermanualTime = millis() - 20000;

  ads.begin();
  ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V

}

boolean supermanualIdle()
{
  return millis() - lastSupermanualTime > 20000;
} 

int adcForBellows[] = {0,2,1,3}; // make slider layout match bellow layout, assuming user is standing behind blower
void loopSupermanual() 
{
  


  gotSupermanual = true;

  for( int i = 0; i < 4; i ++)
  {
      float adc = ads.readADC_SingleEnded(adcForBellows[i]);
      //Serial.println(adc);
      float x = fmap( adc, minAdc, maxAdc, 0.0, 1.0 );



      if( x > 1.1 )
      {
        // normal case when controller is unplugged
        // if( traceSupermanual ) {Serial.println("bad x!");}
        gotSupermanual = false;
        return;
      }
      
      if( fabs(supermanual[i] - x) > 0.05 )
        lastSupermanualTime = millis();
        
      supermanual[i] = x;
      if( traceSupermanual ) {Serial.print(i); Serial.print(":"); Serial.print(x); Serial.print("  ");}
  }

  if( traceSupermanual ) {Serial.println("");}

}
