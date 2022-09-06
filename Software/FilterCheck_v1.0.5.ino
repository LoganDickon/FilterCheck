// Created By: Logan Dickon
// Company: Atorlabs
// Date Created: 3/15/2022
// Date Modified: 3/24/2022

// Licence:

//    This code is for use in the FilterCheck and is owned by Atorlabs. 
//    Permission to duplicate this program must be given by Ator Labs.

// Program Description:  

//    This program uses the TSI5310 FlowMeter w/ serial communication to PID control a 
//    Brushless Fan Blower to 85 SLPM with 0 cmH2O of pressure read on the TSI5310 
//    FlowMeter low pressure sensor. The program then goes into monitor mode until a filter 
//    is detected via the low pressure sensor reading > -.1 cmH2O.

// Device Constants:
  #define PWM           2           // PWM Signal from pin 2 on the Teensy 4.1
  #define METER_RELAY   3           // Relay i/o pin on pin 8 on the Teensy 4.1
  #define FAN_RELAY     4           // Relay i/o pin on pin 6 on the Teensy 4.1
  #define GREEN_LED     5           // Green LED
  #define BLUE_LED      6           // Blue LED
  #define USBSERIAL     Serial      // Rename Serial Main to usbSerial
  #define FLOWMETER     Serial1     // Rename Serial1 to FlowMeter
  #define DEBUG         true        // <<--- truw = SYSTEM DEBUG INFORMATION LOGGED TO SERIAL MONITOR <<---
  #define LED_DEBUG     false       // <<--- true = LED DEBUG INFORMATION LOGGED TO SERIAL MONITOR <<---

// Global Variables:
  double fan_speed = 100, cum_error = 0, previous_error = 0, flow, output, delta_pwm, pressure;
  int i = 0, j = 0, k = 5;
  bool Loop = true;
  unsigned long previousTime = 0;

void setup()
{   
  // Pin Initialization:
    pinMode(PWM, OUTPUT);  // Set PWM pin to OUTPUT mode
    pinMode(METER_RELAY, OUTPUT);  // Set RELAY pin to OUTPUT mode
    pinMode(FAN_RELAY, OUTPUT); // Set RELAY pin to OUTPUT mode
    pinMode(BLUE_LED, OUTPUT);  // Set LED pin to OUTPUT mode
    pinMode(GREEN_LED, OUTPUT);  // Set LED pin to OUTPUT mode

  // Serial Initialization:
    USBSERIAL.begin(115200);  // Start serial for Teensy to PC
    delay(200);
    FLOWMETER.begin(115200);  // Start serial for Teensy to FlowMeter
    delay(200);
    if (DEBUG) {USBSERIAL.println("Serial Initialized!");}

  // Turn On FlowMeter & Fan Blower:
    digitalWrite(BLUE_LED, HIGH);  // Turn off Blue LED
    if (LED_DEBUG) {USBSERIAL.println("Green: 0 | Blue: 1");}
    digitalWrite(METER_RELAY, HIGH);  // Turn Relay HIGH to allow power to the FlowMeter
    if (DEBUG) {USBSERIAL.println("System boot up in 45 seconds!");}
    delay(37500);
    digitalWrite(FAN_RELAY, HIGH);  // Turn on the Brushless Fan Blower
    delay(2500);
    analogWrite(PWM, fan_speed);  // Turn on Fan Blower
    

  // FlowMeter Startup ASCII Commands:
    FLOWMETER.print("SUS\r\n");  // Sets flow measurement to Standard Flow
    delay(500);
    FLOWMETER.print("SG0\r\n");  // Set gas calibration to Air
    delay(500);
    FLOWMETER.print("SSR0010\r\n");  // Set sample rate to 10
    delay(500);

  // Bring Fan Blower To 85 SLPM:
    flow = Read_Sensor("Flow");  // Read the Flow from the TSI 5310 FlowMeter
    
    while (Loop)
    {
      // Setup Loop:
      if (DEBUG) {USBSERIAL.println("Entering Setup Loop!");}
      digitalWrite(BLUE_LED, HIGH);  // Turn on Blue LED
      if (LED_DEBUG) {USBSERIAL.println("Green: 0 | Blue: 1");}
      
      if (fan_speed >= 215)
      { 
        reboot();  // Reboot the entire system
      }  
      
      flow = Read_Sensor("Flow");  // Read the Flow from the TSI 5310 FlowMeter
      delay(500);
      output = computePID(flow);  // Use flow read from FlowMeter as an input to calculate the PID Control
      delta_pwm = output / 1.41667;  // Use the PID output and divide it by 1.41667 to determine what the value the fan speed between 0 - 255 should change to next
      fan_speed += delta_pwm;  // Add change in PWM to the last PWM value to calculate the new PWM value
      analogWrite(PWM, fan_speed);  // Change PWM value to new PWM value
      if (DEBUG) {USBSERIAL.printf("INPUT: %lf | OUTPUT: %lf\n", flow, fan_speed);}
      delay(500);
      
      if (flow < 83.81 || flow > 86.19)
      {
        j = 0;  // Reset Counter to 0
        if (DEBUG) {USBSERIAL.printf("J: %d\n", j);}
      }
      
      if (flow >= 83.81 && flow <= 86.19)
      {
        j++;  // Increment by 1
        if (DEBUG) {USBSERIAL.printf("J: %d\n", j);}
      }
       
      if ((flow >= 83.81 && flow <= 86.19) && j == 5)
      {
        FLOWMETER.print("LPZ\r\n");  // Zeroes Low Pressure Sensor
        Loop = false;
        if (DEBUG) {USBSERIAL.println("Low Pressure Sensor Zeroed!");}
        if (DEBUG) {USBSERIAL.printf("Initialization Complete!: %lf LPM\n", flow);}
      }
      digitalWrite(BLUE_LED, LOW);  // Turn off Blue LED
      if (LED_DEBUG) {USBSERIAL.println("Green: 0 | Blue: 0");}
      delay(350);
    }
}

void loop() 
{
  // Bring Fan Blower To 85 SLPM:
    if (DEBUG) {USBSERIAL.println("Entering Main Loop!");}
    flow = Read_Sensor("Flow");  // Read the Flow from the TSI 5310 FlowMeter
    output = computePID(flow);  // Use flow read from FlowMeter as an input to calculate the PID Control
    delta_pwm = output / 1.41667;  // Use the PID output and divide it by 1.41667 to determine what the value the fan speed between 0 - 255 should change to next
    fan_speed += delta_pwm;  // Add change in PWM to the last PWM value to calculate the new PWM value
    analogWrite(PWM, fan_speed);  // Change PWM value to new PWM value
    delay(500);
    pressure = Read_Sensor("Pressure");  // Read the Pressure from the TSI 5310 FlowMeter
    if (DEBUG) {USBSERIAL.printf("Flow: %lf | Fan Speed: %lf | Pressure: %lf\n", flow, fan_speed, pressure);}
    delay(200);

    if (fan_speed >= 215)
    { 
      reboot();  // Reboot the entire system
    }  
    
    else if (pressure >= -1.8)
    {
      FLOWMETER.print("LPZ\r\n");  // Zeroes Low Pressure Sensor
      if (DEBUG) {USBSERIAL.println("Low Pressure Sensor Zeroed!");}
      digitalWrite(GREEN_LED, LOW);  // Turn off Green LED
      if (LED_DEBUG) {USBSERIAL.println("Green: 0 | Blue: 0");}
      delay(350);
      digitalWrite(GREEN_LED, HIGH);  // Turn on Green LED
      if (LED_DEBUG) {USBSERIAL.println("Green: 1 | Blue: 0");}
    }

    else if (pressure < -1.8)
    {
      digitalWrite(GREEN_LED, LOW);  // Turn off Green LED
      if (LED_DEBUG) {USBSERIAL.println("Green: 0 | Blue: 0");}
      delay(350);
      digitalWrite(GREEN_LED, HIGH); // Turn on Green LED (to make it blink)
      if (LED_DEBUG) {USBSERIAL.println("Green: 1 | Blue: 0");}
    }

    if (flow < 83.81 || flow > 86.19)
    {
      k = 0;  // Reset Counter to 0
    }

    else if ((flow >= 83.81 && flow <= 86.19) && k < 5)
    {
      k++;  // Increment by 1
    }

    while ((flow >= 83.81 && flow <= 86.19) && k == 5)
    {
      flow = Read_Sensor("Flow");  // Read the Flow from the TSI 5310 FlowMeter
      digitalWrite(GREEN_LED, HIGH);  // Turn on Green LED
      if (DEBUG) {USBSERIAL.printf("85 SLPM Achieved!: %lf\n", flow);}
      if (LED_DEBUG) {USBSERIAL.println("Green: 1 | Blue: 0");}
      delay(1500);
    }
}

// Computes the PID (Porportion, Intregal, Derivative) to determine the most effective ΔSpeed to achieve 85 SLPM
double computePID(double inp)
{ 
  // Local Variables:
    unsigned long currentTime;
    double kp = 1, kd = 1, elapsedTime, error, rateError, setPoint = 85.0;
 
  // Compute ElapsedTime:
    currentTime = millis();  // Get current time
    elapsedTime = (double)(currentTime - previousTime);  // Compute time elapsed from previous computation

  // Compute Error, Intergral & Derivative:
    error = setPoint - inp;  // Determine error
    cum_error += error * elapsedTime;  // Compute integral
    rateError = (error - previous_error)/elapsedTime;  // Compute derivative

  // Compute PID Output:
    double out = (kp * error) + (kd * rateError);  // PID output

  // Remember Current Error & Time:
    previous_error = error;  // Remember current error
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
    FLOWMETER.print("DAFxxxxx0003\r\n");  // FlowMeter returns 3 Flow samples to serial buffer
    if (debug) {USBSERIAL.println("Flow Samples Collected");}
    delay(200);
  }

  else if (parameter == "Pressure")
  {
    // Send Pressure Reading ASCII Command to FlowMeter
    FLOWMETER.print("DAxxxxLx0003\r\n");  // FlowMeter returns 3 Pressure samples to serial buffer
    if (debug) {USBSERIAL.println("Pressure Samples Collected");}
    delay(200);
  }

  else 
  {
    if (debug) {USBSERIAL.println("Incorrect Parameter Given To FlowMeter");}
    return 0;
  }
  
  
  // Read Serial Buffer Until Empty:
    while (FLOWMETER.available())
    {
      serial_buf = FLOWMETER.read();  // Read one char of the serial buffer
      serial_buf_String += serial_buf;  // Add the char in the serial buffer to a string
      delay(5);
    }

  // Format serial_buf_String into Flow/Pressure Samples:
    serial_buf_String.replace("OK", "");  // Remove "OK"s from the string
    serial_buf_String.replace("\r", "");  // Remove "\r"s from the string
    serial_buf_String.replace("\n", "");  // Remove "\n"s from the string

    if (debug) {USBSERIAL.println(serial_buf_String);}
    
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

void reboot()
{
  if (DEBUG) {USBSERIAL.println("Entering Reboot Mode");}
  digitalWrite(METER_RELAY, LOW);  // Turn off the the FlowMeter
  digitalWrite(FAN_RELAY, LOW);  // Turn off the Brushless Fan Blower
  delay(50);
  digitalWrite(GREEN_LED, HIGH);  // Turn on Green LED
  digitalWrite(BLUE_LED, HIGH);  // Turn on Blue LED

  if (DEBUG) {USBSERIAL.println("5 second breather for FlowMeter!");}
  delay(5000);
  
  digitalWrite(METER_RELAY, HIGH);  // Turn on the FlowMeter
  if (DEBUG) {USBSERIAL.println("REBOOTING in 45 seconds!");}
  delay(25);
  
  for (int i = 0; i < 150; i++)
  {
    digitalWrite(GREEN_LED, LOW);  // Turn off Green LED
    digitalWrite(BLUE_LED, LOW);  // Turn off Blue LED
    delay(125);
    digitalWrite(GREEN_LED, HIGH);  // Turn on Green LED
    digitalWrite(BLUE_LED, HIGH);  // Turn on Blue LED
    delay(125);
    
  }
  
  fan_speed = 100;
  digitalWrite(FAN_RELAY, HIGH);  // Turn on the Brushless Fan Blower
  delay(2500);
  digitalWrite(GREEN_LED, LOW);  // Turn off Green LED
  digitalWrite(BLUE_LED, LOW);  // Turn off Blue LED
  delay(50);
  analogWrite(PWM, fan_speed);  // Turn on Fan Blower
}
