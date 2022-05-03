// FilterCheck Libraries
  #include <Wire.h>
  #include <AllSensors_DLHR.h>
  #include <sdpsensor.h>
  #include <font_Arial.h>
  #include <font_ArialBlack.h>
  #include "Atorlogo.c"
  #include <ZzzMovingAvg.h>
  #include <StopwatchLib.h>

// Device Constants
  #define BACKLIGHT_PIN   1
  #define TFT_DC          20
  #define TFT_CS          21
  #define TFT_RST         255
  #define TFT_MOSI        7
  #define TFT_SCLK        14
  #define TFT_MISO        12
  #define AtorBlue (11, 84, 139)
  #define PWM             2

// FilterCheck Constructors
  ILI9341_t3 HMI = ILI9341_t3(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK, TFT_MISO);
  SDP8XXSensor flowSensor;
  AllSensors_DLHR_L10D_8 diffSensor(&Wire);
  ZzzMovingAvg <5, float, float> Start_Up_Avg;
  Stopwatch stopwatch;

// Global Variables
  
  // Double
    double diffPressure;
    double zeroedPressure;
    double flowPressure;
    double LPM;
    double mmH2O;
    double mmH2O_temp;
    double timer;
  
  // Integer
    int fanSpeed = 95;
    int flowVal;
    int diffVal;
    int startSpeed = 95;
    int TestCycles = 0;
    int j = 0;
    
  // Boolean
    bool StartTimer = true;
    
void setup() {  // put your setup code here, to run once:
  
  // Communication(s) Setup
    HMI.begin();
    Wire.begin();
    Serial.begin(115200);
    delay(1000);

  // Splash Screen
    Ator_Splash_Screen();
    
  // Pin(s) Setup
    pinMode(13, OUTPUT);
    pinMode(PWM, OUTPUT);
    pinMode(BACKLIGHT_PIN, OUTPUT);
    digitalWrite(13, HIGH);
    analogWrite(PWM, fanSpeed);
    analogWrite(BACKLIGHT_PIN, 255);

    delay(1000);
    
  // Start-up Methods
    Sensor_Val();           // Sensor Validation Method
    Flow_Sensor_Start_Up(); // Stabilization of start-up Flow Method
    Diff_Sensor_Zero();     // Zero out Pressure Sensor Method
    Test_Screen();          // HMI Method for Graphics
}

void loop() {  // put your main code here, to run repeatedly:
  // Run Main Program or Validate Sensors
    flowVal = flowSensor.readSample();
    diffSensor.startMeasurement();
    diffSensor.readData(true);
  
    if (flowSensor.getDifferentialPressure() && diffSensor.pressure) {
      // LPM & Flow Pressure
        flowPressure = flowSensor.getDifferentialPressure();
        LPM = ((((-0.0063 * (flowPressure * flowPressure)) + (1.4899 * flowPressure) + 31.58)));
        HMI.setCursor(65, 90);
        HMI.setFont(Arial_16);
        HMI.fillRect(65, 90, 160, 23, ILI9341_WHITE);
        HMI.setTextColor(ILI9341_BLACK);
        HMI.print(LPM, 2);
        HMI.print(" LPM");

      // Differential Pressure [mmH2O]
        mmH2O = getPressure();
        HMI.setCursor(50, 170);
        HMI.fillRect(20, 170, 200, 40, ILI9341_WHITE);
        HMI.setFont(Arial_16);
        HMI.setTextColor(ILI9341_BLACK);
        HMI.print(mmH2O, 2);
        HMI.print(" mmH2O");
        
        Serial.printf(" Flow: %lf LPM | Fan Speed: %d Bit | Diff Pressure: %lf mmH2O\n", LPM, fanSpeed, mmH2O);

      // Filter Test Procedure
        if (LPM < 83.81) {                                                       
          if (StartTimer == true) {
            stopwatch.Reset();
            StartTimer = false;
            if (LPM > 86.19) {
              StartTimer = true;
              }
              Serial.println("Timer Start!");

          }
          if ((StartTimer == false) && (LPM < 83.81)) {                                           
              delay(50);
              HMI.setFont(Arial_16);
              HMI.setCursor(50, 30);
              HMI.setTextColor(ILI9341_BLACK);                                        
              HMI.println("TESTING...");                                              
              HMI.fillCircle(190, 95, 5, ILI9341_RED);                                
              TestCycles = 0;                                                         
              }
          }
          if ((StartTimer == false) && (LPM > 86.19)) {
            StartTimer = true;
            fanSpeed = startSpeed;
            analogWrite(PWM, fanSpeed);
          }
          if ((StartTimer == false) && ((LPM >= 83.81) && (LPM <= 86.19))) {
            HMI.fillCircle(190, 95, 5, ILI9341_GREEN);                                  
            TestCycles += 1;                                                            
            if (TestCycles == 20) {                                                       
              stopwatch.Update();                                                        
              timer = stopwatch.GetElapsed();                                            
              Serial.print(timer/1000000);                                               
              Serial.print(" Seconds\n");                                                
              StartTimer = true;
              j = 0;                                                         
            }
            while (TestCycles == 20) {
                flowSensor.readSample();
                flowPressure = flowSensor.getDifferentialPressure();
                LPM = ((((-0.0063 * (flowPressure * flowPressure)) + (1.4899 * flowPressure) + 31.58)));
                HMI.fillScreen(ILI9341_WHITE);                                          
                HMI.drawRect(5, 5, 230, 310, AtorBlue);                                 
                HMI.drawRect(6, 6, 227, 307, AtorBlue);                                 
                HMI.drawRect(7, 7, 225, 305, AtorBlue);                                 
                HMI.drawRect(8, 8, 226, 306, AtorBlue);                                 
                HMI.setFont(ArialBlack_24);                                             
                HMI.setCursor(35, 80);                                                  
                HMI.setTextColor(ILI9341_BLACK);                                        
                HMI.setFont(Arial_24);                                                  
                HMI.print("Filter Result \n");                                          
                HMI.setFont(Arial_18);                                                  
                HMI.setCursor(32, 115);                                                 
                HMI.setTextColor(ILI9341_GREEN);                                        
                HMI.print(mmH2O, 2);                                                    
                HMI.print(" mmH2O");                                                   
                Serial.println(LPM);
                if (LPM > 86.2) {
                  Serial.println(startSpeed);
                  analogWrite(PWM, startSpeed);                                         
                  fanSpeed = startSpeed;
                  Test_Screen();                                                        
                  break;
                }
                if (j == 5) {
                  Serial.println(startSpeed);
                  analogWrite(PWM, startSpeed);                                         
                  fanSpeed = startSpeed;
                  Test_Screen();                                                        
                  break;
                }
                j++;
                delay(2000);
              }
          } if ((StartTimer == true) && (LPM <= 80)) {                                           
              HMI.setFont(Arial_16);                                                    
              HMI.setCursor(50, 30);                                                    
              HMI.setTextColor(ILI9341_RED);                                            
              HMI.fillRect(45, 30, 140, 20, ILI9341_WHITE);                             
              HMI.println("NOT READY!");                                                
              delay(600);                                                               
              HMI.fillRect(45, 30, 140, 20, ILI9341_WHITE);                             
          } if ((StartTimer == true) && (LPM > 80)) {                                          
              HMI.setFont(Arial_16);                                                    
              HMI.setCursor(35, 30);                                                    
              HMI.setTextColor(ILI9341_GREEN);                                          
              HMI.fillRect(35, 30, 168, 16, ILI9341_WHITE);                             
              HMI.println("INSERT FILTER!");                                            
              delay(600);                                                               
              HMI.fillRect(35, 30, 168, 16, ILI9341_WHITE);                             
          } if ((StartTimer == false) && (LPM <= 80) && (fanSpeed < 204)) {                      
              fanSpeed += 1.5;                                                          
              analogWrite(PWM, fanSpeed);                                                 
          } if ((StartTimer == false) && (LPM > 80) && (LPM <= 83.81) && (fanSpeed < 204)) {          
              fanSpeed += 1;                                                            
              analogWrite(PWM, fanSpeed);                                               
          } if ((StartTimer == false) && (LPM >= 86.19) && (fanSpeed < 204)) {                      
              fanSpeed -= 1;                                                            
              analogWrite(PWM, fanSpeed);                                               
          } while (fanSpeed >= 204) {
              HMI.fillScreen(ILI9341_RED);                                              
              HMI.drawRect(5, 5, 230, 310, ILI9341_BLACK);                              
              HMI.drawRect(6, 6, 227, 307, ILI9341_BLACK);                              
              HMI.drawRect(7, 7, 225, 305, ILI9341_BLACK);                              
              HMI.drawRect(8, 8, 226, 306, ILI9341_BLACK);                              
              HMI.setCursor(60, 65);                                                    
              HMI.setTextColor(ILI9341_BLACK);                                          
              HMI.setFont(Arial_24);                                                    
              HMI.print("ERROR!");                                                      
              HMI.setCursor(20, 150);                                                   
              HMI.setFont(Arial_14);                                                    
              HMI.print("Remove Clogged Filter");
              fanSpeed = startSpeed;
              analogWrite(PWM, startSpeed);
              delay(5000);
              Test_Screen();
              StartTimer = true;
              }
          } else {                                                         
              Sensor_Val();
              Test_Screen();
        } 
      }

void Sensor_Val() {  // Flow & Diff Sensor Read Validation Method
  // SDP810 Sensor Start Up Validation
      Serial.println("Entering Sensor Validation Loop");
      
      flowVal = flowSensor.init();
      delay(100);
      while (flowVal != 0) {
        flowVal = flowSensor.init();
        Serial.printf("flowVal: %d\n", flowVal);
        delay(100);
        if (flowVal == 0) {
          Serial.println("Exiting Sensor Validation Loop");
          break;
        }
        HMI.fillScreen(ILI9341_RED);
        HMI.drawRect(5, 5, 230, 310, ILI9341_BLACK);
        HMI.drawRect(6, 6, 227, 307, ILI9341_BLACK);
        HMI.drawRect(7, 7, 225, 305, ILI9341_BLACK);
        HMI.drawRect(8, 8, 226, 306, ILI9341_BLACK);
        HMI.setCursor(60, 65);
        HMI.setTextColor(ILI9341_BLACK);
        HMI.setFont(Arial_24);
        HMI.setCursor(10, 150);
        HMI.setFont(Arial_14);
        HMI.print("Flow Sensor Didn't Start!");
        analogWrite(2, 0);
        delay(1000);
      }
  // Amphenol Sensor Start Up Validation
    diffSensor.startMeasurement();
    diffVal = diffSensor.readData(true);
    delay(100);
    while (diffVal != 0) {
        diffSensor.startMeasurement();
        Serial.printf("diffVal: %d\n", diffVal);
        delay(100);
        if (diffVal == 0) {
          Serial.println("Exiting Sensor Validation Loop");
          break;
        }
        HMI.fillScreen(ILI9341_RED);
        HMI.drawRect(5, 5, 230, 310, ILI9341_BLACK);
        HMI.drawRect(6, 6, 227, 307, ILI9341_BLACK);
        HMI.drawRect(7, 7, 225, 305, ILI9341_BLACK);
        HMI.drawRect(8, 8, 226, 306, ILI9341_BLACK);
        HMI.setCursor(60, 65);
        HMI.setTextColor(ILI9341_BLACK);
        HMI.setFont(Arial_24);
        HMI.setCursor(10, 150);
        HMI.setFont(Arial_14);
        HMI.print("Filter Sensor Didn't Start!");
        analogWrite(2, 0);
        delay(1000);
      }
      Serial.println("Exiting Sensor Validation Loop");
}

void Ator_Splash_Screen() {  // HMI Splash Screen Method
  // Create Splash Screen
    HMI.setRotation(4);
    HMI.fillScreen(ILI9341_WHITE);
    HMI.writeRect(15, 75, AtorDrag.width, AtorDrag.height, (uint16_t*)(AtorDrag.pixel_data));
    HMI.drawRect(5, 5, 230, 310, AtorBlue);
    HMI.drawRect(6, 6, 227, 307, AtorBlue);
    HMI.drawRect(7, 7, 225, 305, AtorBlue);
    HMI.drawRect(8, 8, 226, 306, AtorBlue);
    HMI.setCursor(15, 160);
    HMI.setTextColor(AtorBlue);
    HMI.setTextSize(2);
    HMI.setFont(ArialBlack_18);
    HMI.print("FILTER CHECK");
}

void Diff_Sensor_Zero() {  // Differential Pressure Sensor Zeroing Method
  Serial.println("Zeroing Starting!");
  for (int i = 0; i < 8;) {  // Loop Until 12 Pressures Are Read
    diffSensor.startMeasurement();
    diffSensor.readData(true);
    diffPressure = (((0.0101 * (diffSensor.pressure * diffSensor.pressure)) + (0.8826 * diffSensor.pressure) - .0111));
    diffPressure = (diffPressure * 25.374618243743);
    Serial.printf("Pressure: %lf mmH2O\n", diffPressure);
    Serial.printf("Iteration: %d\n", i);
    delay(500);
    if (diffPressure >= -38 && diffPressure <= .1) {
      Start_Up_Avg.add(diffPressure);
      i++;
    }
    zeroedPressure = abs(Start_Up_Avg.get());
    //Serial.println(zeroedPressure);
  }
  Serial.println("Zero Completed!");
}

void Flow_Sensor_Start_Up() { // Stabilize Flow Sensor To 85 LPM At Start Of Program

  Serial.println("Entering Flow Setup");
  bool start_up = true;
  int num = 0;
  
  while (start_up == true) {
    flowVal = flowSensor.readSample();
    flowPressure = flowSensor.getDifferentialPressure();
    LPM = ((((-0.0063 * (flowPressure * flowPressure)) + (1.4899 * flowPressure) + 31.58)));
    Serial.printf("LPM: %lf\n", LPM);
    
    if (LPM <= 84.5) {
      startSpeed += 1;
      analogWrite(PWM, startSpeed);
      Serial.printf("+1: %d\n", startSpeed);
    }
    if (LPM >= 85.5 && startSpeed > 76) {
      startSpeed -= 1;
      analogWrite(PWM, startSpeed);
      Serial.printf("-1: %d\n", startSpeed);
    } 
    if (LPM >= 84.5 && LPM <= 85.5) {
      num++;
      if (num == 3) {
      start_up = false;
      fanSpeed = startSpeed;
      Serial.println(startSpeed);
      Serial.println("Exiting Flow Setup");
      }
    }
    delay(1000);
  }
}

double getPressure() { // Differential Pressure Sensor Reading
  // Get mmH2O From Reading Sensor
    diffSensor.startMeasurement();
    diffSensor.readData(true);
    double pressure = (((0.0101 * (diffSensor.pressure * diffSensor.pressure)) + (0.8826 * diffSensor.pressure) - .0111));
    pressure = ((pressure * 25.374618243743) + zeroedPressure);
    
    return pressure;
}

void Test_Screen() {
  // Create Test Screen
      HMI.fillRect(1, 1, 240, 320, ILI9341_WHITE); 
      HMI.drawRect(5, 5, 230, 310, AtorBlue); 
      HMI.drawRect(6, 6, 227, 307, AtorBlue); 
      HMI.drawRect(7, 7, 225, 305, AtorBlue); 
      HMI.drawRect(8, 8, 226, 306, AtorBlue); 
      HMI.setFont(ArialBlack_24);
      HMI.setCursor(60, 60); 
      HMI.setTextColor(AtorBlue); 
      HMI.setFont(Arial_24); 
      HMI.print("Air Flow \n"); 
      HMI.setCursor(15, 140); 
      HMI.setTextColor(AtorBlue);
      HMI.setFont(Arial_24);
      HMI.print("Filter Pressure \n");
}
