// Created By: Logan Dickon
// Company: Atorlabs
// Date Created: 3/15/2022
// Date Modified: 3/24/2022

// Licence:

//    This code is for use in the FilterCheck and is owned by Atorlabs. 
//    Permission to duplicate this program must be given by Rob Moran and/or Dave Cowgill.

// Program Description:  

//    This program uses the TSI5310 FlowMeter w/ serial communication to PID control a 
//    Brushless Fan Blower to 85 SLPM with 0 cmH2O of pressure read on the TSI5310 
//    FlowMeter low pressure sensor. The program then goes into monitor mode until a filter 
//    is detected via the low pressure sensor reading > -.1 cmH2O.

// Device Constants:
  #define PWM         2           // PWM Signal from pin 2 on the Teensy 3.5
  #define RELAY       8           // Relay i/o pin on pin 8 on the Teensy 3.5
  #define RELAY_2     7           // Relay i/o pin on pin 6 on the Teensy 3.5
  #define BLUE_LED    6           // Blue LED
  #define GREEN_LED   5           // Green LED
  #define usbSerial   Serial      // Rename Serial Main to usbSerial
  #define FlowMeter   Serial1     // Rename Serial1 to FlowMeter
  #define DEBUG       true        // <<--- with this set to true a lot of serial screen output will be seen <<--- \\

// Global Variables:
  double Flow;
  double Flow_Avg = 0;
  int i = 0;
  int j = 0;
  int k = 5;
  bool Loop = true;
  double fan_Speed = 100;
  double input, output;
  double delta_PWM;
  double pressure;
 
void setup()
{   
  // Pin Initialization:
    pinMode(PWM, OUTPUT);  // Set PWM pin to OUTPUT mode
    pinMode(RELAY, OUTPUT);  // Set RELAY pin to OUTPUT mode
    pinMode(RELAY_2, OUTPUT); // Set RELAY pin to OUTPUT mode
    pinMode(BLUE_LED, OUTPUT);  // Set LED pin to OUTPUT mode
    pinMode(GREEN_LED, OUTPUT);  // Set LED pin to OUTPUT mode
    analogWriteFrequency(PWM, 25000);  // Set PWM Frequency to 25KHz

  // Serial Initialization:
    usbSerial.begin(9600);  // Start serial for Teensy to PC
    delay(1000);
    FlowMeter.begin(115200);  // Start serial for Teensy to FlowMeter
    delay(1000);

  // Turn On FlowMeter & Fan Blower:
    analogWrite(PWM, 75);  // Turn On Fan Blower
    digitalWrite(RELAY, HIGH);  // Turn Relay HIGH to allow power to the FlowMeter
    delay(32000);
    //digitalWrite(RELAY_2, HIGH);  // Turn Relay HIGH to allow power to the Brushless Fan Blower

  // FlowMeter Startup ASCII Commands:
    FlowMeter.print("SUS\r\n");  // Sets flow measurement to Standard Flow
    FlowMeter.print("SG0\r\n");  // Set gas calibration to Air
    FlowMeter.print("SSR0005\r\n");  // Set sample rate to 5
    delay(2000);

  // Bring Fan Blower To 85 SLPM:
    input = Read_Sensor("Flow");  // Read the Flow from the TSI 5310 FlowMeter
    
    while (Loop)
    {
      digitalWrite(GREEN_LED, HIGH);  // Turn on Green LED
      input = Read_Sensor("Flow");  // Read the Flow from the TSI 5310 FlowMeter
      output = computePID(input);  // Use flow read from FlowMeter as an input to calculate the PID Control
      delta_PWM = output / 1.41667;  // Use the PID output and divide it by 1.41667 to determine what the value the fan speed between 0 - 255 should change to next
      fan_Speed += delta_PWM;  // Add change in PWM to the last PWM value to calculate the new PWM value
      analogWrite(PWM, fan_Speed);  //Change PWM value to new PWM value
      if (DEBUG) {usbSerial.printf("INPUT: %lf | OUTPUT: %lf\n", input, fan_Speed);}
      delay(600);
      
      if (input < 82 || input > 88)
      {
        j = 0;  // Reset Counter to 0
      }
      
      if (input >= 82 && input <= 88)
      {
        j++;  // Increment by 1
      }
       
      if ((input >= 82 && input <= 88) && j == 5)
      {
        if (DEBUG) {usbSerial.printf("85 SLPM Achieved!: %lf\n", input);}
        if (DEBUG) {usbSerial.println("Low Pressure Sensor Zeroed!");}
        FlowMeter.print("LPZ\r\n"); // Zeroes Low Pressure Sensor
        Loop = false;
      }
       digitalWrite(GREEN_LED, LOW);  // Turn off Green LED
       delay(450);
    }
}

void loop() 
{
  // Bring Fan Blower To 85 SLPM:
      input = Read_Sensor("Flow");  // Read the Flow from the TSI 5310 FlowMeter
      output = computePID(input);  // Use flow read from FlowMeter as an input to calculate the PID Control
      delta_PWM = output / 1.41667;  // Use the PID output and divide it by 1.41667 to determine what the value the fan speed between 0 - 255 should change to next
      fan_Speed += delta_PWM;  // Add change in PWM to the last PWM value to calculate the new PWM value
      analogWrite(PWM, fan_Speed);  //Change PWM value to new PWM value
      delay(200);
      pressure = Read_Sensor("Pressure");  // Read the Pressure from the TSI 5310 FlowMeter
      if (DEBUG) {usbSerial.printf("INPUT: %lf | OUTPUT: %lf\n", input, fan_Speed);}
      delay(200);

      if (pressure >= -.1 && (input >= 82 && input <= 88))
      {
        FlowMeter.print("LPZ\r\n");  // Zeroes Low Pressure Sensor
        digitalWrite(BLUE_LED, LOW);  // Turn off Blue LED
        digitalWrite(GREEN_LED, HIGH);  // Turn on Green LED
      }

      else if (pressure < -.1)
      {
        digitalWrite(GREEN_LED, LOW);  // Turn off Green LED
        digitalWrite(BLUE_LED, LOW);  // Turn off Blue LED
        delay(450);
        digitalWrite(BLUE_LED, HIGH); //Turn on Blue LED (to make it blink)
      }
      
      if (input < 82 || input > 88)
      {
        k = 0;  // Reset Counter to 0
      }

      else if ((input >= 82 && input <= 88) && k < 5)
      {
        k++;  // Increment by 1
      }

      while ((input >= 82 && input <= 88) && k == 5)
      {
        input = Read_Sensor("Flow");  // Read the Flow from the TSI 5310 FlowMeter
        if (DEBUG) {usbSerial.printf("85 SLPM Achieved!: %lf\n", input);}
        if (pressure > -.1)
        {
          digitalWrite(GREEN_LED, HIGH);  // Turn on Green LED
          digitalWrite(BLUE_LED, LOW);  // Turn off Blue LED
        }
        
        if (pressure <= -.1)
        {
          digitalWrite(GREEN_LED, LOW);  // Turn off Green LED
          digitalWrite(BLUE_LED, HIGH);  // Turn on Blue LED
        }
        delay(1500);
      }
}

// Computes the PID (Portportion, Intregal, Derivative) to find the error our fan blower is off to achieve 85 SLPM
double computePID(double inp){
        double kp = 1;
        double ki = 0;
        double kd = 1;
        unsigned long currentTime, previousTime;
        double elapsedTime;
        double error;
        double lastError;
        double cumError, rateError;
        double prevInp = 0;
        double setPoint = 85.0;
       
        // Compute Time:
          currentTime = millis();  // Get current time
          elapsedTime = (double)(currentTime - previousTime);  // Compute time elapsed from previous computation

        // Compute Error, Intergral & Derivative:
          error = setPoint - inp;  // Determine error
          cumError += error * elapsedTime;  // Compute integral
          rateError = (error - lastError)/elapsedTime;  // Compute derivative

        // Compute PID Output:
          double out = kp*error + ki*cumError + kd*rateError;  // PID output               

        // Remember Current Error & Time:
          lastError = error;  // Remember current error
          previousTime = currentTime;  // Remember current time
        
        // Return Output:
          return out;  // Have function return the PID output
}

// Calibrates System to 85 LPM at 0 mmH2O
double Read_Sensor(String parameter)
{
  // Local Variables:
    char serial_buf = "";
    double total = 0;
    double avg;
    float string_to_Float;
    int j = 0;
    String serial_buf_String = "";
    String string_Array[5];
    String total_char = "";
    String array_Data = "";
    bool debug = false;

  if (parameter == "Flow")
  {
    // Send Flow Reading ASCII Command to FlowMeter:
    delay(150);
    FlowMeter.print("DAFxxxxx0005\r\n");  // FlowMeter returns 5 flow samples to serial buffer
    delay(150);
  }

  else if (parameter == "Pressure")
  {
    // Send Pressure Reading ASCII Command to FlowMeter
    delay(150);
    FlowMeter.print("DAxxxxLx0005\r\n");  // FlowMeter returns 5 flow samples to serial buffer
    delay(150);
  }

  else {
    if (debug) {Serial.println("Incorrect Parameter Given To FlowMeter");}
    return 0;
  }
  
  
  // Read Serial Buffer Until Empty:
    while (FlowMeter.available())
    {
      serial_buf = FlowMeter.read();  // Read one char of the serial buffer
      serial_buf_String += serial_buf;  // Add the char in the serial buffer to a string
    }
    
    if (debug) {Serial.println(serial_buf_String);}

  // Format serial_buff_String:
    serial_buf_String.replace("OK", "");  // Remove "OK"s from the string
    serial_buf_String.replace("\r", "");  // Remove "\r"s from the string
    serial_buf_String.replace("\n", "");  // Remove "\n"s from the string
    
  // Convert serial_buf_String to String Array:
    serial_buf_String += ","; // Add a ',' to the end of the string
    for (int i = 0; i < serial_buf_String.length(); i++)
    {
      if ((serial_buf_String[i]) != ',')
      {
        total_char += serial_buf_String[i];  // Add each char to a string until a ',' is read
      }
      if ((serial_buf_String[i]) == ',')
      {
        string_Array[j] = total_char;  // Once ',' is read put string into Array's index
        total_char = "";  // Clear the string to allow new data to be stored
        j++;  // Each time a ',' is read increase j by 1 to goto the next Array index
      }
    }
  
  // Convert Each Array Index From String To Float:
    for (int p = 0; p < 5; p++)
    {
      array_Data = string_Array[p];  // Store data from a specific array index
      string_to_Float = array_Data.toFloat();  // Convert the array data from String to Float
      total += string_to_Float;  // Add up all array data from each index
    }

  // Calculate Average Flow & Return Flow
    avg = (total / 5);  // Divide total by the number of array indexes
    return avg;  // Return Flow Data
}
