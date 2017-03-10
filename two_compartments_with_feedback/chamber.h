class Chamber {
   public:
  int pumpPin;
  int valvePin;
  int sensorPin;
  float pressure;
  float targetPressure;
  float deadband;
  float maxPressure;

 
  Chamber(int p, int v, int s);
  void pumpSpeed(float p);
  void readPressure();
  void setTargetPressure( float t );
  void setTargetFraction(float f);  // 0 to 1.0
  void loop();
};

