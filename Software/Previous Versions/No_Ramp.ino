 #include <Wire.h>
#include <AllSensors_DLHR.h>
#include <sdpsensor.h>
#include <ZzzMovingAvg.h>

SDP8XXSensor flowSensor;
AllSensors_DLHR_L10D_8 diffSensor(&Wire);
ZzzMovingAvg <5, float, float> Start_Up_Avg;
  
int flowVal;
int startSpeed = 80;
double flowPressure;
double LPM;
double mmH2O;
double zeroedPressure;
double diffPressure;

#define PWM 2

void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  Serial.begin(115200);

  Flow_Sensor_Start_Up();
  Diff_Sensor_Zero();
  
}

void loop() {
  // put your main code here, to run repeatedly:
  analogWrite(PWM, 85);
  mmH2O = getPressure();
  flowVal = flowSensor.readSample();
  flowPressure = flowSensor.getDifferentialPressure();
  LPM = ((((-0.00004 * (flowPressure * flowPressure)) + (.0329 * flowPressure) + 1.6349)) * 28.316846999);
  Serial.printf("Pressure: %lf | LPM: %lf | Fan: %d\n", mmH2O, LPM, startSpeed);
  delay(500);
}

void Flow_Sensor_Start_Up() { // Stabilize Flow Sensor To 85 LPM At Start Of Program

  Serial.println("Entering Flow Setup");
  bool start_up = true;
  
  while (start_up == true) {
    flowVal = flowSensor.readSample();
    flowPressure = flowSensor.getDifferentialPressure();
    LPM = ((((-0.00004 * (flowPressure * flowPressure)) + (.0329 * flowPressure) + 1.6349)) * 28.316846999);
    
    if (LPM <= 84.5) {
      startSpeed += 1;
      analogWrite(PWM, startSpeed);
    } if (LPM >= 85.5) {
      startSpeed -= 1;
      analogWrite(PWM, startSpeed);
    } if (LPM >= 84.5 && LPM <= 85.5) {
      start_up = false;
      Serial.println(startSpeed);
      Serial.println(LPM);
      Serial.println("Exiting Flow Setup");
    } 
    delay(100);
  }
}

double getPressure() { // Differential Pressure Sensor Reading
  // Get mmH2O From Reading Sensor
    diffSensor.startMeasurement();
    diffSensor.readData(true);
    double pressure = (diffSensor.pressure * 25.374618243743) + zeroedPressure;
  return pressure;
}

void Diff_Sensor_Zero() {  // Differential Pressure Sensor Zeroing Method
  Serial.println("Zeroing Starting!");
  for (int i = 0; i < 12;) {  // Loop Until 12 Pressures Are Read
    diffSensor.readData(true);
    diffPressure = (diffSensor.pressure * 25.374618243743);
    Serial.printf("Pressure: %lf mmH2O\n", diffPressure);
    Serial.printf("Iteration: %d\n", i);
    delay(500);
    if (diffPressure >= -6 && diffPressure <= 6) {
      Start_Up_Avg.add(diffPressure);
      i++;
    }
    zeroedPressure = abs(Start_Up_Avg.get());
    //Serial.println(zeroedPressure);
  }
  Serial.println("Zero Completed!");
}
