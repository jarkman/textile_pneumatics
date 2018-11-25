class Reservoir {
   public:
  int pumpPin;
  int sensorPin;
  float pressure;
  float targetPressure;
  float deadband;
  float smoothedPressure;
  float pumpSpeed;
  float pumpDelta;

 
  Reservoir(int p, int s);

  void readPressure();
  void setTargetPressure( float t );

   void setPumpSpeed(float p);

  void loop();
};
