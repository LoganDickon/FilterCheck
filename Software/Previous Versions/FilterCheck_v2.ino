/* Filter Tester - Program controls 
fan speed with LPM variable to calculate 
differnetial pressure PSI at the mask */

/* NOTE All HMI Class Calls Are For Screen Creation On The ILI9341 */

// Filter Tester Libraries
  #include <Wire.h>                   // Wire library
  #include <sdpsensor.h>              // SDP sensor library
  #include <MS5525DSO.h>              // MS5525DSO library
  #include <font_Arial.h>             // ILI9341  Arial Font Library
  #include <font_ArialBlack.h>        // ILI9341  Arial Bold Font Library
  #include "Atorlogo.c"               // AtorLABS Splash Image
  #include <ZzzMovingAvg.h>           // Sensor Smoothing Library
  #include <StopwatchLib.h>           // Stopwatch Library // 

// Constant Definitions
  // ILI9341 HMI Pin Definitions
    #define BACKLIGHT_PIN   1
    #define TFT_DC          20
    #define TFT_CS          21
    #define TFT_RST         255
    #define TFT_MOSI        7
    #define TFT_SCLK        14
    #define TFT_MISO        12
  
  // ASCii RGB Definitions
    #define AtorBlue (11, 84, 139)

// Library Constructors
  ILI9341_t3 HMI = ILI9341_t3(TFT_CS, TFT_DC, TFT_RST, TFT_MOSI, TFT_SCLK, TFT_MISO);       // Call ILI9341's constructor
  SDP8XXSensor flowSensor;                                                                  // Call SDP sensor's constructor
  MS5525DSO diffSensor(pp001DS);                                                            // Call MS5525DSO sensor's constructor
  ZzzMovingAvg <5, float, float> avg;                                                       // SDP Sensor Smoothing Constructor
  ZzzMovingAvg <5, float, float> avg2;                                                      // MS5525DSO Sensor Smoothing Constructor
  Stopwatch stopwatch;                                                                      // Serial Stopwatch Constructor // 

// Initialize Variables
  // Double
    double flowPressure;          // SDP Pressure
    double AvgFlow;               // flowPressure Smoothed
    double AvgPressure;           // startPressure Smoothed
    double diffPressure;          // MS5525DSO Pressure
    double LPM;                   // flowPressure converted to flow
    double mmH2O;                 // diffPressure converted to mmH2O
    double mmH2O_2;               // diffPressure zeroed out
    double mmH2O_3;               // diffPressure for reset
    double fanSpeed = 80;        // PWM Fan Speed
    double timer;                 // Holds Stopwatch Value // 
  // Integer
    int SDPVal;                   // Returned Value of SDP810 Pressure Validation Sequence
    int MSVal;                    // Returned Value of MS5525DSO Pressure Validation Sequence
    int TestCycles = 0;           // Number of Successful Tests In A Row
    int i;                        // StartCheck Loop Iteration
  // Boolean
    bool StartTimer = true;       // Control Timer // 

// Main Setup BEGIN
  void setup() // put your setup code here, to run once:
  {
    // Device Setups
      HMI.begin();                                                    // Begin SPI Comm Link With ILI9341 Display
      Wire.begin();                                                   // Begin i2c Comm Link
      Serial.begin(115200);                                           // Begin Serial Comm Link // 
  
    // Pin Setups
      pinMode(35, OUTPUT);                                             // PWM FAN CONTROL
      pinMode(BACKLIGHT_PIN, OUTPUT);                                 // ILI9341 BACKLIGHT CONTROL
    
    // Adjust HMI backlight
      analogWrite(BACKLIGHT_PIN, 255);                                // Set Backlight Brightness
      analogWrite(35, fanSpeed);                                       // Set Initial Fan Speed
      SDPVal = flowSensor.init();                                     // Initialize SDP810 Sensor
      MSVal = diffSensor.begin(I2C_MS5525DSO_ADDR_ALT);               // Initialize MS5525DSO Sensor
      delay(1000);                                                    // 1 Sec Delay To Allow All Initializations To Complete And Stabilize

    // Program Methods
      Sensor_Val();                                                   // Check if sensors started up
      Ator_Splash_Screen();                                           // HMI Screen Splash Screen
      i = 0;                                                          // reset i to 0
      while (i < 12) {
        diffSensor.readPressureAndTemperature(&diffPressure);         // Read MS5525DSO Sensors PSI
        if (diffPressure >= -.035 && diffPressure <= .035) {
          avg2.add(diffPressure);                                     // Add Last Read Pressure To Smoother
          i++;                                                        // Add 1 to i
        }
        AvgPressure = avg2.get();                                     // Return Last Average
        mmH2O_2 = abs(AvgPressure * 703.08893732448);                 // Take absolute value of mmH2O_2 & convert from psi to mmH2O
        delay(200);                                                   // Allow Sensor To Read
        Serial.printf("i: %d\n", i);
        Serial.println(mmH2O_2);                                      // 
        Serial.println(AvgPressure);                                  // 
      }
      Test_Screen();                                                  // Generate Second HMI Screen
  }
// Main Setup END

// Main Loop BEGIN
  void loop() // put your main code here, to run repeatedly:
  {
    SDPVal = flowSensor.readSample();                                                                                   // Attempt to read from SDP810 sensor
    //delay(100);
    if (flowSensor.getDifferentialPressure() && diffSensor.readPressureAndTemperature(&diffPressure)) {                 // If SDP sensor gives a Pressure readout
      // Flow Pressure
        flowPressure = flowSensor.getDifferentialPressure();                                                            // Read SDP Sensors Pa Output
        avg.add(flowPressure);                                                                                          // Add flowPressure to smoother
        AvgFlow = avg.get();                                                                                            // Return Last Calculated Average
        LPM = ((((-0.00004 * (AvgFlow * AvgFlow)) + (.0329 * AvgFlow) + 1.6349)) * 28.316846999);                       // Calculation For LPM Conversion
        HMI.setCursor(65, 90);                                                                                          // HMI Commands
        HMI.setFont(Arial_16);                                                                                          //
        HMI.fillRect(65, 90, 160, 23, ILI9341_WHITE);                                                                   //
        HMI.setTextColor(ILI9341_BLACK);                                                                                //
        HMI.print(LPM, 2);                                                                                              //
        HMI.print(" LPM");                                                                                              //
      
      // Differential Pressure
        diffSensor.readPressureAndTemperature(&diffPressure);                                                           // Read MS5525DSO Sensors PSI
        mmH2O = ((diffPressure * 703.08893732448) - mmH2O_2);                                                           // Convert PSI To mmH2O
        HMI.setCursor(50, 170);                                                                                         // HMI Commands
        HMI.fillRect(20, 170, 200, 40, ILI9341_WHITE);                                                                  //
        HMI.setFont(Arial_16);                                                                                          //
        HMI.setTextColor(ILI9341_BLACK);                                                                                //
        HMI.print(mmH2O, 2);                                                                                            //
        HMI.print(" mmH2O");                                                                                            //
        Serial.printf(" Flow: %lf LPM | Fan Speed: %lf Bit | Diff Pressure: %lf mmH2O | MS5525DSO: %d | SDP810: %d\n", LPM, fanSpeed, mmH2O, MSVal, SDPVal); // 
        
      // Main Test Sequence
        if (mmH2O <= -3) {                                                       
            if (StartTimer == true) {                                                 // 
              stopwatch.Reset();                                                      // 
              StartTimer = false;                                                     //
              diffSensor.readPressureAndTemperature(&diffPressure);                   // Read MS5525DSO Sensors PSI
              mmH2O = ((diffPressure * 703.08893732448) - mmH2O_2);                   // Convert PSI To mmH2O
              if (mmH2O >= -3) {
                StartTimer = true;
              }
              Serial.println("Timer Start!");                                         // 
          } if (LPM < 83.81 || LPM > 86.19) {                                           
              HMI.setFont(Arial_16);                                                  // HMI Commands
              HMI.setCursor(50, 30);                                                  //
              HMI.setTextColor(ILI9341_BLACK);                                        //
              HMI.println("TESTING...");                                              //
              HMI.fillCircle(190, 95, 5, ILI9341_RED);                                //
              TestCycles = 0;                                                         // Reset TestCycles To 0
          }
        }
        if (mmH2O <= -3 && LPM >= 83.81 && LPM <= 86.19) {
          HMI.fillCircle(190, 95, 5, ILI9341_GREEN);                                  // HMI Commands
          TestCycles += 1;                                                            // Iterate TestCycle By 1 Each Cycle
          if (TestCycles == 5) {                                                      // 
            stopwatch.Update();                                                       // 
            timer = stopwatch.GetElapsed();                                           // 
            Serial.print(timer/1000000);                                              // 
            Serial.print(" Seconds\n");                                               // 
            StartTimer = true;                                                        // 
          }
          while (mmH2O <= -3 && TestCycles == 5) {
              diffSensor.readPressureAndTemperature(&diffPressure);                   // Read MS5525DSO Sensors PSI
              mmH2O_3 = ((diffPressure * 703.08893732448) - mmH2O_2);                 // Convert PSI To mmH2O
              HMI.fillScreen(ILI9341_WHITE);                                          // HMI Commands
              HMI.drawRect(5, 5, 230, 310, AtorBlue);                                 //
              HMI.drawRect(6, 6, 227, 307, AtorBlue);                                 //
              HMI.drawRect(7, 7, 225, 305, AtorBlue);                                 //
              HMI.drawRect(8, 8, 226, 306, AtorBlue);                                 //
              HMI.setFont(ArialBlack_24);                                             //
              HMI.setCursor(35, 80);                                                  //
              HMI.setTextColor(ILI9341_BLACK);                                        //
              HMI.setFont(Arial_24);                                                  //
              HMI.print("Filter Result \n");                                          //
              HMI.setFont(Arial_18);                                                  //
              HMI.setCursor(32, 115);                                                 //
              HMI.setTextColor(ILI9341_GREEN);                                        //
              HMI.print(mmH2O, 2);                                                    //
              HMI.print(" mmH2O");                                                    //
              Serial.println(mmH2O_3);
              Serial.println(fanSpeed);
              delay(2000);                                                            // 2 Sec Delay
              if (mmH2O_3 > -3) {
                fanSpeed = 80;                                                       // Reset Fan Blower To 110 Bit
                analogWrite(35, fanSpeed);                                             // Fan Blower Adjustment
                Test_Screen();                                                        // Second Screen Method Call
                break;
              }
            }
        } if (LPM <= 80 && mmH2O > -3) {                                           
            HMI.setFont(Arial_16);                                                    // HMI Commands
            HMI.setCursor(50, 30);                                                    //
            HMI.setTextColor(ILI9341_RED);                                            //
            HMI.fillRect(45, 30, 140, 20, ILI9341_WHITE);                             //
            HMI.println("NOT READY!");                                                //
            delay(600);                                                               // .6 Sec Delay
            HMI.fillRect(45, 30, 140, 20, ILI9341_WHITE);                             //
        } if (LPM > 80 && mmH2O > -3) {                                          
            HMI.setFont(Arial_16);                                                    // HMI Commands
            HMI.setCursor(35, 30);                                                    //
            HMI.setTextColor(ILI9341_GREEN);                                          //
            HMI.fillRect(35, 30, 168, 16, ILI9341_WHITE);                             //
            HMI.println("INSERT FILTER!");                                            //
            delay(600);                                                               // .6 Sec Delay
            HMI.fillRect(35, 30, 168, 16, ILI9341_WHITE);                             //
        } if (LPM <= 80 && mmH2O <= -3 && fanSpeed < 255) {                      
            fanSpeed += 1.5;                                                          // Iterate Fan Blower By 1.5
            analogWrite(35, fanSpeed);                                                 // Fan Blower Adjustment
        } if (LPM > 80 && LPM <= 83.81 && mmH2O <= -3 && fanSpeed < 255) {          
            fanSpeed += 1;                                                            // Iterate Fan Blower By 1
            analogWrite(35, fanSpeed);                                                 // Fan Blower Adjustment
        } if (LPM >= 86.19 && mmH2O <= -3) {                      
            fanSpeed -= 1;                                                            // Iterate Fan Blower By -1
            analogWrite(35, fanSpeed);                                                 // Fan Blower Adjustment
        } if (mmH2O > -3) {
            fanSpeed = 80;                                                           // Reset Fan Blower To 110 Bit
            analogWrite(35, fanSpeed);                                                 // Fan Blower Adjustment
        } while (fanSpeed >= 204 & mmH2O <= -3) {
            HMI.fillScreen(ILI9341_RED);                                              //
            HMI.drawRect(5, 5, 230, 310, ILI9341_BLACK);                              //
            HMI.drawRect(6, 6, 227, 307, ILI9341_BLACK);                              //
            HMI.drawRect(7, 7, 225, 305, ILI9341_BLACK);                              //
            HMI.drawRect(8, 8, 226, 306, ILI9341_BLACK);                              //
            HMI.setCursor(60, 65);                                                    //
            HMI.setTextColor(ILI9341_BLACK);                                          //
            HMI.setFont(Arial_24);                                                    //
            HMI.print("ERROR!");                                                      //
            HMI.setCursor(20, 150);                                                   //
            HMI.setFont(Arial_14);                                                    //
            HMI.print("Remove Clogged Filter");                                       //
            fanSpeed = 80;                                                           // Reset Fan Blower To 110 Bit
            analogWrite(35, fanSpeed);                                                 // Fan Blower Adjustment
            delay(6000);
            Test_Screen();
            }
        } else {                                                         
            Sensor_Val();                                                             // Sensor_Val Method Call
            if (SDPVal == 0 && MSVal == 1) {
              Test_Screen();}                                                         // Test_Screen Method Call
        }
    }
// Main Loop END

// HMI SplashScreen Method BEGIN
  void Ator_Splash_Screen() // HMI Splash Screen Function
  {
    // Create Splash Screen
      HMI.setRotation(2);                                                                             // HMI Commands
      HMI.fillScreen(ILI9341_WHITE);                                                                  //
      HMI.writeRect(15, 75, AtorDrag.width, AtorDrag.height, (uint16_t*)(AtorDrag.pixel_data));       // ATORLOGO
      HMI.drawRect(5, 5, 230, 310, AtorBlue);                                                         //
      HMI.drawRect(6, 6, 227, 307, AtorBlue);                                                         //
      HMI.drawRect(7, 7, 225, 305, AtorBlue);                                                         //
      HMI.drawRect(8, 8, 226, 306, AtorBlue);                                                         //
      HMI.setCursor(15, 160);                                                                         //
      HMI.setTextColor(AtorBlue);                                                                     //
      HMI.setTextSize(2);                                                                             //
      HMI.setFont(ArialBlack_18);                                                                     //
      HMI.print("FILTER CHECK");                                                                      //
      delay(5000);                                                                                    // 5 Sec Delay
  }
// HMI SplashScreen Method END

// HMI TestScreen Method BEGIN
  void Test_Screen() 
  {
    // Create Second Screen
      HMI.fillRect(1, 1, 240, 320, ILI9341_WHITE);        // HMI Commands
      HMI.drawRect(5, 5, 230, 310, AtorBlue);             //
      HMI.drawRect(6, 6, 227, 307, AtorBlue);             //
      HMI.drawRect(7, 7, 225, 305, AtorBlue);             //
      HMI.drawRect(8, 8, 226, 306, AtorBlue);             //
      HMI.setFont(ArialBlack_24);                         //
      HMI.setCursor(60, 60);                              //
      HMI.setTextColor(AtorBlue);                         //
      HMI.setFont(Arial_24);                              //
      HMI.print("Air Flow \n");                           //
      HMI.setCursor(15, 140);                             //
      HMI.setTextColor(AtorBlue);                         //
      HMI.setFont(Arial_24);                              //
      HMI.print("Filter Pressure \n");                    //
  }
// HMI TestScreen Method END

// Sensor Validation Method BEGIN
  void Sensor_Val()
  {
    // SDP810 Sensor Start Up Validation
      Serial.println("ENTERING VALIDATION LOOP");
      while (SDPVal != 0) {
        delay(100);                                                             // 100 milli Second Delay
        SDPVal = flowSensor.init();                                             // Attempt To Start SDP Sensor
        Serial.printf("SDP810: %d\n", SDPVal);
        if (SDPVal == 0) {
          break;                                                                // Skip Loop Iteration
        }
        HMI.setRotation(2);                                                     // HMI Commands
        HMI.fillScreen(ILI9341_RED);                                            //
        HMI.drawRect(5, 5, 230, 310, ILI9341_BLACK);                            //
        HMI.drawRect(6, 6, 227, 307, ILI9341_BLACK);                            //
        HMI.drawRect(7, 7, 225, 305, ILI9341_BLACK);                            //
        HMI.drawRect(8, 8, 226, 306, ILI9341_BLACK);                            //
        HMI.setCursor(60, 65);                                                  //
        HMI.setTextColor(ILI9341_BLACK);                                        //
        HMI.setFont(Arial_24);                                                  //
        HMI.setCursor(10, 150);                                                 //
        HMI.setFont(Arial_14);                                                  //
        HMI.print("Flow Sensor Didn't Start!");                                 //
        analogWrite(2, 0);                                                      // Turn Fan Blower Off
        delay(1000);                                                            // 1 Sec Delay
      }
      // MS5525DSO Sensor Start Up Validation
      while (MSVal != 1) {
        delay(100);                                                             // 100 milli Second Delay
        MSVal = diffSensor.begin(I2C_MS5525DSO_ADDR_ALT);                       // Attempt To Start MS5525DSO Sensor
        Serial.printf("MS5525DSO: %d\n", MSVal);
        if (MSVal == 0) {
          break;                                                                // Skip Loop Iteration
        }
        HMI.setRotation(2);                                                     // HMI Commands
        HMI.fillScreen(ILI9341_RED);                                            //
        HMI.drawRect(5, 5, 230, 310, ILI9341_BLACK);                            //
        HMI.drawRect(6, 6, 227, 307, ILI9341_BLACK);                            //
        HMI.drawRect(7, 7, 225, 305, ILI9341_BLACK);                            //
        HMI.drawRect(8, 8, 226, 306, ILI9341_BLACK);                            //
        HMI.setCursor(60, 65);                                                  //
        HMI.setTextColor(ILI9341_BLACK);                                        //
        HMI.setFont(Arial_24);                                                  //
        HMI.setCursor(10, 150);                                                 //
        HMI.setFont(Arial_14);                                                  //
        HMI.print("Filter Sensor Didn't Start!");                               //
        analogWrite(2, 0);                                                      // Turn Fan Blower Off
        delay(1000);                                                            // 1 Sec Delay
      }
  }
// End of Sensor_Val()
