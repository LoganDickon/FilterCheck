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
  #include "Atorlogo.c"              // AtorLABS Splash Image
  #include <ZzzMovingAvg.h>           // Sensor Smoothing Library
  #include <StopwatchLib.h>           // Stopwatch Library // COMMENT OUT FOR FINAL USE!

// Constant Definitions
  // ILI9341 HMI Pin Definitions
  #define BACKLIGHT_PIN   3 
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
  ZzzMovingAvg <5, float, float> avg;                                                       // Sensor Smoothing Constructor
  Stopwatch stopwatch;                                                                      // Serial Stopwatch Constructor// COMMENT OUT FOR FINAL USE!

// Initialize Variables
  // Double
  double flowPressure;          // SDP Pressure
  double AvgFlow;               // flowPressure Smoothed 
  double diffPressure;          // MS5525DSO Pressure
  double LPM;                   // flowPressure converted to flow
  double mmH2O;                 // diffPressure converted to mmH2O
  double fanSpeed = 110;        // PWM Fan Speed
  double timer;                 // Holds Stopwatch Value // COMMENT OUT FOR FINAL USE!
  // Integer
  int SDPVal;                   // Returned value of SDP Pressure Validation Sequence
  int Switch;                   // Switch ON/OFF
  int TestCycles = 0;           // Number of Successful Tests In A Row
  // Boolean
  bool StartTimer = true;       // Control Timer // COMMENT OUT FOR FINAL USE!

// Main Setup BEGIN
  void setup() // put your setup code here, to run once:
  {
    // Device Setups
      HMI.begin();                            // Begin SPI Comm Link With ILI9341 Display
      Wire.begin();                           // Begin i2c Comm Link
      Serial.begin(115300);                   // Begin Serial Comm Link // COMMENT OUT FOR FINAL USE!
  
    // Pin Setups
      pinMode(2, OUTPUT);                     // PWM FAN CONTROL
      pinMode(4, OUTPUT);                     // FILTER INSERTION SWITCH
      pinMode(BACKLIGHT_PIN, OUTPUT);         // ILI9341 BACKLIGHT CONTROL
    
    // Adjust HMI backlight
      analogWrite(BACKLIGHT_PIN, 255);        // Set Backlight Brightness
      analogWrite(2, fanSpeed);               // Set Initial Fan Speed
      SDPVal = flowSensor.init();             // Initialize SDP Sensor
      delay(1000);                            // 1 Sec Delay To Allow All Initializations To Complete And Stabilize
    
    // Program Methods
      Sensor_Val();                           // Check if sensors started up
      Ator_Splash_Screen();                   // HMI Screen Splash Screen
      Second_Screen();                        // Generate Second HMI Screen
  }
// Main Setup END

// Main Loop BEGIN
  void loop() // put your main code here, to run repeatedly:
  {
    SDPVal = flowSensor.readSample();                                                                               // Attempt to read from SDP sensor
    if (SDPVal == 0 && diffSensor.begin(I2C_MS5525DSO_ADDR_ALT)) {                                                  // If SDP sensor gives a Pressure readout
      // Flow Pressure
        flowPressure = flowSensor.getDifferentialPressure();                                                        // Read SDP Sensors Pa Output
        AvgFlow = avg.get();                                                                                        // Get Average SDP Sensor Pa Value
        LPM = ((((-0.00004 * (AvgFlow * AvgFlow)) + (.0329 * AvgFlow) + 1.6349)) * 28.316846999);                   // Calculation For LPM Conversion
        Serial.printf("Real Pascal = %lf | Average Pascal = ", flowPressure);                                       // COMMENT OUT FOR FINAL USE!
        Serial.print(avg.add(flowPressure));                                                                        // COMMENT OUT FOR FINAL USE!
        Serial.printf(" | Flow = %lf LPM | Fan Speed = %lf Bit | Test # = %d \n", LPM, fanSpeed, TestCycles);       // COMMENT OUT FOR FINAL USE!
        HMI.setCursor(65, 90);                                                                                      // HMI Commands
        HMI.setFont(Arial_16);                                                                                      //
        HMI.fillRect(65, 90, 160, 23, ILI9341_WHITE);                                                               //
        HMI.setTextColor(ILI9341_BLACK);                                                                            //
        HMI.print(LPM, 2);                                                                                          //
        HMI.print(" LPM");                                                                                          //
      
      // Differential Pressure
        diffSensor.readPressureAndTemperature(&diffPressure);       // Read MS5525DSO Sensors PSI
        mmH2O = diffPressure * 703.08893732448 + 33;                // Convert PSI To mmH2O
        HMI.setCursor(50, 170);                                     // HMI Commands
        HMI.fillRect(20, 170, 200, 40, ILI9341_WHITE);              //
        HMI.setFont(Arial_16);                                      //
        HMI.setTextColor(ILI9341_BLACK);                            //
        HMI.print(mmH2O, 2);                                        //
        HMI.print(" mmH2O");                                        //
  
      // Main Test Sequence
        Switch = digitalRead(4);                                                      // Read Switch ON/OFF
        if (Switch == HIGH && LPM >= 83.6 && LPM <= 86.4) {                           
          HMI.fillCircle(190, 95, 5, ILI9341_GREEN);                                  // HMI Commands
          TestCycles += 1;                                                            // Iterate TestCycle By 1 Each Cycle
          if (TestCycles == 3) {                                                      // COMMENT OUT FOR FINAL USE!
            stopwatch.Update();                                                       // COMMENT OUT FOR FINAL USE!
            timer = stopwatch.GetElapsed();                                           // COMMENT OUT FOR FINAL USE!
            Serial.print(timer/1000000);                                              // COMMENT OUT FOR FINAL USE!
            Serial.print(" Seconds\n");                                               // COMMENT OUT FOR FINAL USE!
            StartTimer = true;                                                        // COMMENT OUT FOR FINAL USE!
          }
          Switch = digitalRead(4);                                                    // Read Switch ON/OFF
          while (Switch == HIGH && TestCycles == 3) {
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
              HMI.setTextColor(ILI9341_GREENYELLOW);                                  //
              HMI.print(mmH2O, 2);                                                    //
              HMI.print(" mmH2O");                                                    //
              delay(2000);                                                            // 2 Sec Delay
              Switch = digitalRead(4);                                                // Read Switch ON/OFF
              if (Switch == LOW) {
                fanSpeed = 110;                                                       // Reset Fan Blower To 110 Bit
                analogWrite(2, fanSpeed);                                             // Fan Blower Adjustment
                Second_Screen();                                                      // Second Screen Method Call
                break;}}
        } if (Switch == HIGH) {                                                       
          if (StartTimer == true) {                                                   // COMMENT OUT FOR FINAL USE!
            Serial.println("Timer Start!");                                           // COMMENT OUT FOR FINAL USE!
            stopwatch.Reset();                                                        // COMMENT OUT FOR FINAL USE!
            StartTimer = false;                                                       // COMMENT OUT FOR FINAL USE!
          } if (LPM < 83.6 || LPM > 86.4) {                                           
              HMI.setFont(Arial_16);                                                  // HMI Commands
              HMI.setCursor(50, 30);                                                  //
              HMI.setTextColor(ILI9341_BLACK);                                        //
              HMI.println("TESTING...");                                              //
              HMI.fillCircle(190, 95, 5, ILI9341_RED);                                //
              TestCycles = 0;}                                                        // Reset TestCycles To 0
        } if (LPM <= 80 && Switch == LOW) {                                           
            HMI.setFont(Arial_16);                                                    // HMI Commands
            HMI.setCursor(50, 30);                                                    //
            HMI.setTextColor(ILI9341_RED);                                            //
            HMI.fillRect(45, 30, 140, 20, ILI9341_WHITE);                             //
            HMI.println("NOT READY!");                                                //
            delay(600);                                                               // .6 Sec Delay
            HMI.fillRect(45, 30, 140, 20, ILI9341_WHITE);                             //
          } if (LPM > 80 && Switch == LOW) {                                          
              HMI.setFont(Arial_16);                                                  // HMI Commands
              HMI.setCursor(35, 30);                                                  //
              HMI.setTextColor(ILI9341_GREEN);                                        //
              HMI.fillRect(35, 30, 168, 16, ILI9341_WHITE);                           //
              HMI.println("INSERT FILTER!");                                          //
              delay(600);                                                             // .6 Sec Delay
              HMI.fillRect(35, 30, 168, 16, ILI9341_WHITE);                           //
          } if (LPM <= 80 && Switch == HIGH && fanSpeed < 255) {                      
              fanSpeed += 1.5;                                                        // Iterate Fan Blower By 1.5
              analogWrite(2, fanSpeed);                                               // Fan Blower Adjustment
          } if (LPM > 80 && LPM <= 83 && Switch == HIGH && fanSpeed < 255) {          
              fanSpeed += 1;                                                          // Iterate Fan Blower By 1
              analogWrite(2, fanSpeed);                                               // Fan Blower Adjustment
          } if (LPM >= 86 && Switch == HIGH && fanSpeed > 100) {                      
              fanSpeed -= 1;                                                          // Iterate Fan Blower By -1
              analogWrite(2, fanSpeed);                                               // Fan Blower Adjustment
          } if (Switch == LOW) {
            fanSpeed = 110;                                                           // Reset Fan Blower To 110 Bit
            analogWrite(2, fanSpeed);}                                                // Fan Blower Adjustment
          } else {                                                                    
              Sensor_Val();                                                           // Sensor_Val Method Call
              if (SDPVal == 0 && diffSensor.begin(I2C_MS5525DSO_ADDR_ALT)) {          // If Both SDP & MS5525DSO Are Initialized
                Second_Screen();}                                                     // Second_Screen Method Call
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

// HMI SecondScreen Method BEGIN
  void Second_Screen() 
  {
    // Create Second Screen
      HMI.fillRect(1, 1, 240, 320, ILI9341_WHITE);        // HMI Commands
      HMI.drawRect(5, 5, 230, 310, AtorBlue);             //
      HMI.drawRect(6, 6, 227, 307, AtorBlue);             //
      HMI.drawRect(7, 7, 225, 305, AtorBlue);             //
      HMI.drawRect(8, 8, 226, 306, AtorBlue);             //
      HMI.setFont(ArialBlack_24);                         //
      HMI.setCursor(60, 60);                              //
      HMI.setTextColor(ILI9341_BLACK);                    //
      HMI.setFont(Arial_24);                              //
      HMI.print("Air Flow \n");                           //
      HMI.setCursor(15, 140);                             //
      HMI.setTextColor(ILI9341_BLACK);                    //
      HMI.setFont(Arial_24);                              //
      HMI.print("Filter Pressure \n");                    //
  }
// HMI SecondScreen Method END

// Sensor Validation Method BEGIN
  void Sensor_Val()
  {
    // SDP & MS5525DSO Sensor Start Up Validation
      while (SDPVal != 0 || !diffSensor.begin(I2C_MS5525DSO_ADDR_ALT)) {        // Otherwise Throw Error Code
        HMI.setRotation(2);                                                     // HMI Commands
        HMI.fillScreen(ILI9341_RED);                                            //
        HMI.setCursor(60, 65);                                                  //
        HMI.setTextColor(ILI9341_BLACK);                                        //
        HMI.setFont(Arial_24);                                                  //
        HMI.print("ERROR!");                                                    //
        HMI.setCursor(10, 150);                                                 //
        HMI.setFont(Arial_14);                                                  //
        HMI.print("Sensor(s) Failed To Start!");                                //
        analogWrite(2, 0);                                                      // Turn Fan Blower Off
        SDPVal = flowSensor.init();                                             // Attempt To Start SDP Sensor
        delay(1000);}                                                           // 1 Sec Delay
  }
// End of Sensor_Val()
