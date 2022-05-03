// Created By: Logan Dickon
// Company: Atorlabs
// Date Created: 3/15/2022
// Date Modified: 3/16/2022

// Device Constants:
  #define PWM         2           // PWM Signal from pin 2 on the Teensy 3.5
  #define RELAY       8           // Relay i/o pin from pin 8 on the Teensy 3.5
  #define LED         7           // Blue LED
  #define usbSerial   Serial      // Rename Serial Main to usbSerial
  #define FlowMeter   Serial1     // Rename Serial1 to FlowMeter
  #define DEBUG       true        // <<--- with this set to true a lot of serial screen output will be seen
                                  // an easy way to debug progress;  

// Global Variables:
  double Flow;
  double Flow_Avg = 0;
  int i = 0;
  int j = 0;
  bool Loop = true;
  double fan_Speed = 180;
  double input, output;
  double delta_PWM;
 
void setup()
{   
  // Pin Initialization:
    pinMode(PWM, OUTPUT);  // Set PWM pin to OUTPUT mode
    pinMode(RELAY, OUTPUT);  // Set RELAY pin to OUTPUT mode
    pinMode(LED, OUTPUT);  // Set LED pin to OUTPUT mode

  // Serial Initialization:
    usbSerial.begin(9600);  // Start serial for Teensy to PC
    delay(1000);
    FlowMeter.begin(115200);  // Start serial for Teensy to FlowMeter
    delay(1000);

  // Turn On FlowMeter & Fan Blower:
    digitalWrite(PWM, fan_Speed);  // Turn On Fan Blower
    digitalWrite(RELAY, HIGH);  // Turn Relay HIGH to allow power to the FlowMeter
    delay(26000);

  // FlowMeter Startup ASCII Commands:
    FlowMeter.print("SUS\r\n");  // Sets flow measurement to Standard Flow
    FlowMeter.print("SG0\r\n");  // Set gas calibration to Air
    FlowMeter.print("SSR0005\r\n");  // Set sample rate to 5
    delay(1000);

  // Bring Fan Blower To 85 SLPM:
    input = Read_Flow();
    while (Loop)
    {
      input = Read_Flow();
      output = computePID(input);
      delta_PWM = output / 1.4166666666666666666666666666667;
      fan_Speed += delta_PWM;
      analogWrite(PWM, fan_Speed);
      if (DEBUG) {usbSerial.printf("INPUT: %lf | OUTPUT: %lf\n", input, fan_Speed);}
      
      delay(500);
      
      if (input < 82 || input > 88)
      {
        j = 0;
      }
      
      if (input >= 82 && input <= 88)
      {
        j++;
      }
       
      if ((input >= 82 && input <= 88) && j == 5)
      {
        if (DEBUG) {usbSerial.printf("85 SLPM Achieved!: %lf\n", input);}
        if (DEBUG) {usbSerial.println("Low Pressure Sensor Zeroed!");}
        FlowMeter.print("LPZ\r\n"); // Zeroes Low Pressure Sensor
        Loop = false;
      }
    }
}

void loop() 
{
  // Bring Fan Blower To 85 SLPM:
      input = Read_Flow();
      output = computePID(input);
      delta_PWM = output / 1.4166666666666666666666666666667;
      fan_Speed += delta_PWM;
      analogWrite(PWM, fan_Speed);
      
      if (DEBUG) {usbSerial.printf("INPUT: %lf | OUTPUT: %lf\n", input, fan_Speed);}
      
      delay(500);
      
      if (input < 82 || input > 88)
      {
        j = 0;
      }
      
      if (input >= 82 && input <= 88)
      {
        j++;
      }
       
      if ((input >= 82 && input <= 88) && j == 5)
      {
        if (DEBUG) {usbSerial.printf("85 SLPM Achieved!: %lf\n", input);}
      }
}

// Calibrates System to 85 LPM at 0 mmH2O
double Read_Flow()
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
  
  // FlowMeter Read ASCII Command:
    FlowMeter.print("DAFxxxxx0005\r\n"); // FlowMeter returns 5 flow samples to serial buffer
    delay(150);
  
  // Read Serial Buffer Until Empty:
    while (FlowMeter.available())
    {
      serial_buf = FlowMeter.read(); // Read one char of the serial buffer
      serial_buf_String += serial_buf; // Add the char in the serial buffer to a string
    }

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
        string_Array[j] = total_char; // Once ',' is read put string into Array's index
        total_char = ""; // Clear the string to allow new data to be stored
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
    return avg; // Return Flow Data
}

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
          error = setPoint - inp; // Determine error
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
