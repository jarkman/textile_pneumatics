class Chamber {
   public:
  int inflatePin;
  int deflatePin;
  int sensorPin;
  float pressure;
  float targetPressure;
  float deadband;
  int state;
  float smoothedPressure;
  float pulseFactor;
  int remainingMs;

 
  Chamber(int i, int v, int s);
  void readPressure();
  void setTargetPressure( float t );
  void setTargetFraction(float f);  // 0 to 1.0
  
  void loop();
  void oneMsTimer();
};

