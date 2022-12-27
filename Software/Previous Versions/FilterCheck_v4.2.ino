// Created By: Logan Dickon
// Company: ATOR LABS
// Date Created: 3/15/2022
// Date Modified: 10/13/2022

// Licence:

//    This code is for use in the FilterCheck and is owned by ATOR LABS. 
//    Do not disperse this Software unless given permission by ATOR LABS.

// Program Description:

//    This program uses the TSI5310 FlowMeter w/ serial communication to PID control a 
//    Brushless Fan Blower to 85 SLPM with 0 cmH2O of pressure read on the TSI5310 
//    FlowMeter low pressure sensor. The program then goes into monitor mode until a filter 
//    is detected via the low pressure sensor reading > -.1 cmH2O.

// Device Constants:
  #define PWM         2           // PWM Signal from pin 2 on the Teensy 3.5
  #define RELAY       3           // Relay i/o pin on pin 8 on the Teensy 3.5
  #define RELAY_2     4           // Relay i/o pin on pin 6 on the Teensy 3.5
  #define GREEN_LED   5           // Green LED
  #define BLUE_LED    6           // Blue LED
  #define usbSerial   Serial      // Rename Serial Main to usbSerial
  #define FlowMeter   Serial1     // Rename Serial1 to FlowMeter
  #define DEBUG       true        // <<--- True = SYSTEM Debug Info
  #define LED_DEBUG   true        // <<--- True = LED Debug Info

// Global Variables:
  double Flow, Flow_Avg = 0;
  int i = 0, j = 0, k = 5;
  bool Loop = true;
  double fan_Speed = 100, input, output, delta_PWM, pressure;

void setup()
{   
  // Pin Initialization:
    pinMode(PWM, OUTPUT);  // Set PWM pin to OUTPUT mode
    pinMode(RELAY, OUTPUT);  // Set RELAY pin to OUTPUT mode
    pinMode(RELAY_2, OUTPUT); // Set RELAY pin to OUTPUT mode
    pinMode(BLUE_LED, OUTPUT);  // Set LED pin to OUTPUT mode
    pinMode(GREEN_LED, OUTPUT);  // Set LED pin to OUTPUT mode

  // Serial Initialization:
    usbSerial.begin(9600);  // Start serial for Teensy to PC
    delay(200);
    FlowMeter.begin(115200);  // Start serial for Teensy to FlowMeter
    delay(200);

  // Turn On FlowMeter & Fan Blower:
    digitalWrite(BLUE_LED, HIGH);  // Turn on the Blue LED
    digitalWrite(RELAY, HIGH);  // Turn Relay HIGH to allow 5v to the FlowMeter
    delay(40000);
    digitalWrite(RELAY_2, HIGH);  // Turn Relay HIGH to allow 24v to the Brushless Fan Blower
    delay(2500);
    analogWrite(PWM, fan_Speed);  // Turn on the Fan Blower
    

  // FlowMeter Startup ASCII Commands:
    FlowMeter.print("SUS\r\n");  // Pre-sets the readings to be in Standard Flow mode
    delay(500);
    FlowMeter.print("SG0\r\n");  // Pre-sets the FlowMeter's Calibration to Air
    delay(500);
    FlowMeter.print("SSR0010\r\n");  // Pre-sets the Sample Rate of the readings to be 10
    delay(500);

  // Bring Fan Blower To 85 SLPM:
    input = Read_Sensor("Flow");  // Read the Flow from the FlowMeter
    
    while (Loop)
    {
      digitalWrite(BLUE_LED, HIGH);  // Turn on the Blue LED
      if (LED_DEBUG) {usbSerial.println("Green: 0 | Blue: 1");}
      input = Read_Sensor("Flow");  // Read the Flow from the FlowMeter
      delay(500);
      output = computePID(input);  // Compute the PID using the Flow reading
      delta_PWM = output / 1.41667;  // Use the PID output and divide it by 1.41667 to determine what the value the fan speed between 0 - 255 should change to next
      fan_Speed += delta_PWM;  // Add change in PWM to the last PWM value to calculate the new PWM value
      analogWrite(PWM, fan_Speed);  // Change PWM value to new PWM value
      if (DEBUG) {usbSerial.printf("INPUT: %lf | OUTPUT: %lf\n", input, fan_Speed);}
      delay(500);
      
      if (input < 83.81 || input > 86.19)
      {
        j = 0;  // Reset Counter to 0
        if (DEBUG) {usbSerial.printf("J: %d\n", j);}
      }
      
      if (input >= 83.81 && input <= 86.19)
      {
        j++;  // Increment by 1
        if (DEBUG) {usbSerial.printf("J: %d\n", j);}
      }
       
      if ((input >= 83.81 && input <= 86.19) && j == 5)
      {
        FlowMeter.print("LPZ\r\n");  // Zeroes Low Pressure Sensor
        Loop = false;
        if (DEBUG) {usbSerial.println("Low Pressure Sensor Zeroed!");}
        if (DEBUG) {usbSerial.printf("Initialization Complete!: %lf LPM\n", input);}
      }
      digitalWrite(BLUE_LED, LOW);  // Turn off Blue LED
      if (LED_DEBUG) {usbSerial.println("Green: 0 | Blue: 0");}
      delay(250);
    }
}

void loop() 
{
  // Bring Fan Blower To 85 SLPM:
    input = Read_Sensor("Flow");  // Read the Flow from the TSI 5310 FlowMeter
    output = computePID(input);  // Use flow read from FlowMeter as an input to calculate the PID Control
    delta_PWM = output / 1.41667;  // Use the PID output and divide it by 1.41667 to determine what the value the fan speed between 0 - 255 should change to next
    fan_Speed += delta_PWM;  // Add change in PWM to the last PWM value to calculate the new PWM value
    analogWrite(PWM, fan_Speed);  // Change PWM value to new PWM value
    delay(200);
    pressure = Read_Sensor("Pressure");  // Read the Pressure from the TSI 5310 FlowMeter
    if (DEBUG) {usbSerial.printf("INPUT: %lf | OUTPUT: %lf\n", input, fan_Speed);}
    delay(200);

    if (pressure >= -1.8 && (input >= 83.81 && input <= 86.19))
    {
      FlowMeter.print("LPZ\r\n");  // Zeroes Low Pressure Sensor
      digitalWrite(GREEN_LED, HIGH);  // Turn off Green LED
      if (DEBUG) {usbSerial.println("Low Pressure Sensor Zeroed!");}
      if (LED_DEBUG) {usbSerial.println("Green: 1 | Blue: 0");}
    }

    else if (pressure < -1.8  && (input >= 70 && input <= 86.19))
    {
      digitalWrite(GREEN_LED, LOW);  // Turn off Green LED
      if (LED_DEBUG) {usbSerial.println("Green: 0 | Blue: 0");}
      delay(350);
      digitalWrite(GREEN_LED, HIGH); // Turn on Green LED (to make it blink)
      if (LED_DEBUG) {usbSerial.println("Green: 1 | Blue: 0");}
    }

    if (input < 83.81 || input > 86.19)
    {
      k = 0;  // Reset Counter to 0
    }

    else if ((input >= 83.81 && input <= 86.19) && k < 5)
    {
      k++;  // Increment by 1
    }

    while ((input >= 83.81 && input <= 86.19) && k == 5)
    {
      input = Read_Sensor("Flow");  // Read the Flow from the TSI 5310 FlowMeter
      digitalWrite(GREEN_LED, HIGH);  // Turn on Green LED
      if (DEBUG) {usbSerial.printf("85 SLPM Achieved!: %lf\n", input);}
      if (LED_DEBUG) {usbSerial.println("Green: 1 | Blue: 0");}
      delay(1500);
    }
}

// Computes the PID (Porportion, Intregal, Derivative) to determine the most effective ΔSpeed to achieve 85 SLPM
double computePID(double inp)
{ 
  // Local Variables:
    unsigned long currentTime, previousTime;
    double kp = 1, kd = 1, elapsedTime, error, lastError, cumError, rateError, setPoint = 85.0;
 
  // Compute ElapsedTime:
    currentTime = millis();  // Get current time
    elapsedTime = (double)(currentTime - previousTime);  // Compute time elapsed from previous computation

  // Compute Error, Intergral & Derivative:
    error = setPoint - inp;  // Determine error
    cumError += error * elapsedTime;  // Compute integral
    rateError = (error - lastError)/elapsedTime;  // Compute derivative

  // Compute PID Output:
    double out = (kp * error) + (kd * rateError);  // PID output

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
    char serial_buf;
    double total = 0, avg_Sample;
    float string_to_Float;
    int j = 0, num_of_Samples = 3;
    String serial_buf_String = "", string_Array[5], total_char = "", array_Data = "";
    bool debug = false;

  if (parameter == "Flow")
  {
    // Send Flow Reading ASCII Command to FlowMeter:
    FlowMeter.print("DAFxxxxx0003\r\n");  // FlowMeter returns 3 Flow samples to serial buffer
    if (debug) {usbSerial.println("Flow Samples Collected");}
    delay(200);
  }

  else if (parameter == "Pressure")
  {
    // Send Pressure Reading ASCII Command to FlowMeter
    FlowMeter.print("DAxxxxLx0003\r\n");  // FlowMeter returns 3 Pressure samples to serial buffer
    if (debug) {usbSerial.println("Pressure Samples Collected");}
    delay(200);
  }

  else 
  {
    if (debug) {usbSerial.println("Incorrect Parameter Given To FlowMeter");}
    return 0;
  }
  
  
  // Read Serial Buffer Until Empty:
    while (FlowMeter.available())
    {
      serial_buf = FlowMeter.read();  // Read one char of the serial buffer
      serial_buf_String += serial_buf;  // Add the char in the serial buffer to a string
      delay(5);
    }

  // Format serial_buf_String into Flow/Pressure Samples:
    serial_buf_String.replace("OK", "");  // Remove "OK"s from the string
    serial_buf_String.replace("\r", "");  // Remove "\r"s from the string
    serial_buf_String.replace("\n", "");  // Remove "\n"s from the string

    if (debug) {usbSerial.println(serial_buf_String);}
    
  // Convert serial_buf_String to String Array:
    serial_buf_String += ",";  // Add a ',' to the end of the string
    for (unsigned int i = 0; i < serial_buf_String.length(); i++)
    {
      if ((serial_buf_String[i]) != ',')
      {
        total_char += serial_buf_String[i];  // Add each char to a string until a ',' is read
      }
      else if ((serial_buf_String[i]) == ',')
      {
        string_Array[j] = total_char;  // Once ',' is read put string into Array's index
        total_char = "";  // Clear the string to allow new data to be stored
        j++;  // Each time a ',' is read increase j by 1 to goto the next Array index
      }
    }
  
  // Convert Each Array Index From String To Float:
    for (int p = 0; p < 3; p++)
    {
      array_Data = string_Array[p];  // Store array index p to a string variable
      string_to_Float = array_Data.toFloat();  // Convert the string variable to a float variable
      total += string_to_Float;  // Add float variable to a running total
    }

  // Calculate Average Flow & Return Flow
    avg_Sample = (total / num_of_Samples);  // Calculate the average sample total by dividing the running total by the # of samples returned 
    return avg_Sample;  // Return average 
}
