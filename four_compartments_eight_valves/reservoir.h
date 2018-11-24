class Reservoir {
   public:
  int pumpPin;
  int sensorPin;
  float pressure;
  float targetPressure;
  float deadband;
  float smoothedPressure;


 
  Reservoir(int p, int s);

  void readPressure();
  void setTargetPressure( float t );

   void pumpSpeed(float p);

  void loop();
};
