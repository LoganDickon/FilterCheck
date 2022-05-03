#include <Wire.h>

#include <AllSensors_DLHR.h>

AllSensors_DLHR_L10D_8 gagePressure(&Wire);

double diffPressure = 0;
double mmH2O = 0;
double inH2O_cal = 0;
double mmH2O_cal = 0;
double inH2O = 0;

void setup() {
  Serial.begin(115200);
  Wire.begin();
  delay(1000);
  
  pinMode(2, OUTPUT);
  analogWrite(2, 85);
  gagePressure.setPressureUnit(AllSensors_DLHR::PressureUnit::IN_H2O);
  gagePressure.startMeasurement();
  delay(1000);
}

void loop() {
  gagePressure.readData(true);
  inH2O = gagePressure.pressure;
  mmH2O = (gagePressure.pressure * 25.374618243743);
  mmH2O_cal = (((0.0101 * (inH2O * inH2O)) + (0.8826 * inH2O) - .0111));
  mmH2O_cal = (mmH2O_cal * 25.374618243743);
  Serial.printf("Pressure: %lf inH2O | Pressure: %lf mmH2O | Cal Pressure: %lf mmH2O\n", inH2O, mmH2O, mmH2O_cal);
  delay(1000);
}
