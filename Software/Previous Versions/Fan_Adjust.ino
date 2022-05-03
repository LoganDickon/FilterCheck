#include <Wire.h>
#include <sdpsensor.h>

SDP8XXSensor flowSensor;

double flowPressure;
int flowVal;

#define PWM 2
void setup() {
  // put your setup code here, to run once:
  Wire.begin();
  Serial.begin(115200);
  
  pinMode(PWM, OUTPUT);
  analogWrite(PWM, 205);
}

void loop() {
    // put your main code here, to run repeatedly:
    flowVal = flowSensor.readSample();
    flowPressure = flowSensor.getDifferentialPressure();
    Serial.println(flowPressure);
    delay(200);
}
