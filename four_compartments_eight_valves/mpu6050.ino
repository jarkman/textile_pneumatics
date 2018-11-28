

// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"
#include "MPU6050.h"

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
    #include "Wire.h"
#endif

// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 accelgyro;
//MPU6050 accelgyro(0x69); // <-- use for AD0 high

int16_t ax, ay, az;
int16_t gx, gy, gz;

float xAngle = 0.0, yAngle = 0.0, zAngle = 0.0; // measures of recent angle change
float returnRate = 20.0; // degs/sec

long lastMicros;

// uncomment "OUTPUT_READABLE_ACCELGYRO" if you want to see a tab-separated
// list of the accel X/Y/Z and then gyro X/Y/Z values in decimal. Easy to read,
// not so easy to parse, and slow(er) over UART.
//#define OUTPUT_READABLE_ACCELGYRO

// uncomment "OUTPUT_BINARY_ACCELGYRO" to send all 6 axes of data as 16-bit
// binary, one right after the other. This is very fast (as fast as possible
// without compression or data loss), and easy to parse, but impossible to read
// for a human.
//#define OUTPUT_BINARY_ACCELGYRO


void setupMpu6050() {
    // join I2C bus (I2Cdev library doesn't do this automatically)
    #if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
        Wire.begin();
    #elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
        Fastwire::setup(400, true);
    #endif

    // initialize device
    Serial.println("Initializing I2C devices...");
    accelgyro.initialize();

    accelgyro.setDMPEnabled(false);

    // verify connection
    Serial.println("Testing device connections...");
    if( ! accelgyro.testConnection())
    {
      gotImu = false;
      Serial.println( "MPU6050 connection failed");
      return;
    }
    else
    {
      gotImu = true;
      Serial.println( "MPU6050 connection successful");
     
    }


    // use the code below to change accel/gyro offset values
    /*
    Serial.println("Updating internal sensor offsets...");
    // -76  -2359 1688  0 0 0
    Serial.print(accelgyro.getXAccelOffset()); Serial.print("\t"); // -76
    Serial.print(accelgyro.getYAccelOffset()); Serial.print("\t"); // -2359
    Serial.print(accelgyro.getZAccelOffset()); Serial.print("\t"); // 1688
    Serial.print(accelgyro.getXGyroOffset()); Serial.print("\t"); // 0
    Serial.print(accelgyro.getYGyroOffset()); Serial.print("\t"); // 0
    Serial.print(accelgyro.getZGyroOffset()); Serial.print("\t"); // 0
    Serial.print("\n");
    accelgyro.setXGyroOffset(220);
    accelgyro.setYGyroOffset(76);
    accelgyro.setZGyroOffset(-85);
    Serial.print(accelgyro.getXAccelOffset()); Serial.print("\t"); // -76
    Serial.print(accelgyro.getYAccelOffset()); Serial.print("\t"); // -2359
    Serial.print(accelgyro.getZAccelOffset()); Serial.print("\t"); // 1688
    Serial.print(accelgyro.getXGyroOffset()); Serial.print("\t"); // 0
    Serial.print(accelgyro.getYGyroOffset()); Serial.print("\t"); // 0
    Serial.print(accelgyro.getZGyroOffset()); Serial.print("\t"); // 0
    Serial.print("\n");
    */

    // configure Arduino LED pin for output
    pinMode(LED_PIN, OUTPUT);
}

// Maintain a measure of recent angle change
float updateAngle( float a, int16_t g, float secs )
{
  float degsPerSec = (float) g / 131.0;
  float degs = degsPerSec * secs;
  a = a + degs;

  float returnDelta = returnRate * secs; // bringing us back to zero at fixed rate
  if( a > returnDelta )
    a -= returnDelta;
  else if( a < -returnDelta )
    a += returnDelta;

  a = fconstrain( a, -180.0, 180.0 );

/*
  Serial.print(degsPerSec);
  Serial.print(", ");
  Serial.println(a);
 */ 
  return a;
    
}


void loopMpu6050() {
  if( trace ) Serial.println("accelgyro.getMotion6");
    // read raw accel/gyro measurements from device
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
  if( trace ) Serial.println("accelgyro.getMotion6 done");

    // numbers in the range +- 32000 or so
    // ax around 10k is a nod forward/back
    // gz around +20000 is a head turn left, -20000 is a head turn right

    /* possible scale factors
     *  16 384 counts/g  
        8 192 counts/g  
        4 096 counts/g  
        2 048 counts/g  

      Angular Velocity Limit  |   Sensitivity (counts per deg/s)
      ----------------------------------------
      250º/s                  |    131
      500º/s                  |    65.5 
      1000º/s                 |    32.8 
      2000º/s                 |    16.4
     */


    long now = micros();

    float secs = 0.000001 * (float) (now - lastMicros);
    lastMicros = now;

    secs = fconstrain( secs, 0.0, 0.010 ); 
    
    xAngle = updateAngle( xAngle, gx, secs );
    yAngle = updateAngle( yAngle, gy, secs );
    zAngle = updateAngle( zAngle, gz, secs );

    // +ve x is head lean right
    // +ve y is nod forwards
    // +ve z is head turn left
    
    imuNod = yAngle/30.0; // nod forwards
    imuTurn = -zAngle/30.0; // turn to the right
    imuLean = xAngle / 30.0;
    
    // these methods (and a few others) are also available
    //accelgyro.getAcceleration(&ax, &ay, &az);
    //accelgyro.getRotation(&gx, &gy, &gz);

    #ifdef OUTPUT_READABLE_ACCELGYRO
        // display tab-separated accel/gyro x/y/z values
        Serial.print("a/g:\t");
        Serial.print(ax); Serial.print("\t");
        Serial.print(ay); Serial.print("\t");
        Serial.print(az); Serial.print("\t");
        Serial.print(gx); Serial.print("\t");
        Serial.print(gy); Serial.print("\t");
        Serial.println(gz);
    #endif

    #ifdef OUTPUT_BINARY_ACCELGYRO
        Serial.write((uint8_t)(ax >> 8)); Serial.write((uint8_t)(ax & 0xFF));
        Serial.write((uint8_t)(ay >> 8)); Serial.write((uint8_t)(ay & 0xFF));
        Serial.write((uint8_t)(az >> 8)); Serial.write((uint8_t)(az & 0xFF));
        Serial.write((uint8_t)(gx >> 8)); Serial.write((uint8_t)(gx & 0xFF));
        Serial.write((uint8_t)(gy >> 8)); Serial.write((uint8_t)(gy & 0xFF));
        Serial.write((uint8_t)(gz >> 8)); Serial.write((uint8_t)(gz & 0xFF));
    #endif

    
}
