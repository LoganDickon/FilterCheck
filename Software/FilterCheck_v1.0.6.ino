// Created By: Logan Dickon
// Company: Atorlabs
// Date Created: 3/15/2022
// Date Modified: 10/18/2022

// Licence:

//    This code is for use in the FilterCheck and is owned by Atorlabs. 
//    Permission to duplicate this program must be given by Ator Labs.

// Program Description:  

//    This program uses the TSI5310-2 flow meter w/ serial communication to PID control a 
//    Brushless Fan Blower to 85 SLPM with 0 cmH2O of pressure read on the TSI5310-2 
//    FlowMeter low-pressure sensor. The program then goes into monitor mode until a filter 
//    is detected via the low-pressure sensor reading > -.8 cmH2O.

// Device Constants:
  #define PWM           2           // PWM Signal on pin 2 on the Teensy 4.1
  #define METER_RELAY   3           // Relay i/o on pin 3 on the Teensy 4.1
  #define FAN_RELAY     4           // Relay i/o on pin 4 on the Teensy 4.1
  #define GREEN_LED     5           // Green LED
  #define BLUE_LED      6           // Blue LED
  #define USBSERIAL     Serial      // Rename Serial Main to usbSerial
  #define FLOWMETER     Serial1     // Rename Serial1 to FlowMeter
  #define DEBUG         true        // <<--- true = SYSTEM DEBUG INFORMATION LOGGED TO SERIAL MONITOR <<---

// Global Variables:
  double fan_speed = 100, cum_error = 0, previous_error = 0, flow, output, delta_pwm, pressure;
  int i = 0, j = 0, k = 3, counter = 0, pressure_counter = 0;
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
    delay(1000);
    digitalWrite(METER_RELAY, HIGH);  // Turn Relay HIGH to allow power to the flow meter
    if (DEBUG == true) {
      USBSERIAL.println("System boot up in 40 seconds!");
      delay(10000);
      USBSERIAL.println("System boot up in 30 seconds!");
      delay(15000);
      USBSERIAL.println("System boot up in 15 seconds!");
      delay(10000);
      USBSERIAL.println("System boot up in 5 seconds!");
      delay(5000);
      USBSERIAL.println("System Booting!");
    }
    else if (DEBUG == false)
    {
      delay(40000);
    }

    digitalWrite(FAN_RELAY, HIGH);  // Turn on the BLDC Fan Blower
    delay(2500);
    analogWrite(PWM, fan_speed);  // Change the BLDC Fan Blower's speed
    

  // FlowMeter Startup ASCII Commands:
    FLOWMETER.print("SUS\r\n");  // Sets flow measurement to Standard Flow
    delay(500);
    FLOWMETER.print("SG0\r\n");  // Set gas calibration to Air
    delay(500);
    FLOWMETER.print("SSR0010\r\n");  // Set sample rate to 10
    delay(500);

    // Setup Loop:
      while (Loop)
      {
        if (DEBUG) {USBSERIAL.println("\nEntering Setup Loop!");}
        digitalWrite(BLUE_LED, HIGH);  // Turn on Blue LED
  
        // Watchdog code to protect the FilterCheck against hardware failure
          if (fan_speed > 200)
          {
            Reboot();
            fan_speed = 100;
            analogWrite(PWM, fan_speed);  // Change PWM value to new PWM value
            delay(1000);
          }
  
        // Obtain the ΔSpeed that the BLDC Fan Blower needs to change by to obtain 85 SLPM of flow
          flow = Read_Sensor("Flow");  // Read the Flow from the TSI 5310-2 flow meter
          delay(350);
          output = Compute_PID(flow);  // Input flow to the PID Method to calculate the fan speed error 
          //delta_pwm = output / 1.41667;  // Use the PID output and divide it by 1.41667 to determine what the value the fan speed between 0 - 255 should change to next
          //fan_speed += delta_pwm;  // Add change in PWM to the last PWM value to calculate the new PWM value
          fan_speed += output;  // Add change in PWM to the last PWM value to calculate the new PWM value
          analogWrite(PWM, fan_speed);  // Change the BLDC Fan Blower's speed
          if (DEBUG) {USBSERIAL.printf("INPUT: %lf | OUTPUT: %lf\n", flow, fan_speed);}
          delay(250);

        // Verify the flow rate is 85 SLPM by using a watchdog Counter mechanism
          if (flow < 83.81 || flow > 86.19)
          {
            j = 0;  // Reset the Counter
            if (DEBUG) {USBSERIAL.printf("J: %d\n", j);}
          }
          
          else if (flow >= 83.81 && flow <= 86.19)
          {
            j++;  // Increment the Counter
            if (DEBUG) {USBSERIAL.printf("J: %d\n", j);}
          }
           
          if ((flow >= 83.81 && flow <= 86.19) && j == 3)
          {
            FLOWMETER.print("LPZ\r\n");  // Zeroes Low Pressure Sensor
            Loop = false;
            if (DEBUG) {USBSERIAL.println("Low Pressure Sensor Zeroed!");}
            if (DEBUG) {USBSERIAL.printf("Initialization Complete!: %lf LPM\n", flow);}
          }
        
        digitalWrite(BLUE_LED, LOW);  // Turn off Blue LED (Makes it blink!)
        delay(250);
      }
}

void loop() 
{
  // Watchdog code to protect the FilterCheck against hardware failure
    if (fan_speed > 200)
    {
      Reboot();
      fan_speed = 100;
      analogWrite(PWM, fan_speed);  // Change PWM value to new PWM value
      delay(1000);
    }
  
  // Bring Fan Blower To 85 SLPM:
    if (DEBUG) {USBSERIAL.println("\nEntering Main Loop!");}
    flow = Read_Sensor("Flow");  // Read the Flow from the TSI 5310-2 flow meter
    output = Compute_PID(flow);  // Use flow read from FlowMeter as an input to calculate the PID Control
    //delta_pwm = output / 1.41667;  // Use the PID output and divide it by 1.41667 to determine what the value the fan speed between 0 - 255 should change to next
    //fan_speed += delta_pwm;  // Add change in PWM to the last PWM value to calculate the new PWM value
    fan_speed += output;  // Add change in PWM to the last PWM value to calculate the new PWM value
    analogWrite(PWM, fan_speed);  // Change the BLDC Fan Blower's speed
    delay(200);
    pressure = Read_Sensor("Pressure");  // Read the Pressure from the TSI 5310-2 flow meter
    if (DEBUG) {USBSERIAL.printf("Flow: %lf | Fan Speed: %lf | Pressure: %lf\n", flow, fan_speed, pressure);}
    delay(200);
  
  // Blink LED's & Zero the Low-Pressure Sensor if there no filter is on
    if (pressure >= -.7 && (flow >= 83.81 && flow <= 120))
    {
      pressure_counter += 1;
      if (pressure_counter == 3)
      {
        FLOWMETER.print("LPZ\r\n");  // Zeroes Low Pressure Sensor
        if (DEBUG) {USBSERIAL.println("Low Pressure Sensor Zeroed!");}
      }
      digitalWrite(GREEN_LED, LOW);  // Turn off Green LED
      delay(250);
      digitalWrite(GREEN_LED, HIGH);  // Turn on Green LED (to make it blink)
    }

  // Blink LED's if there is a filter on
    else if (pressure < -.8  && (flow >= 70 && flow <= 86.19))
    {
      pressure_counter = 0;
      digitalWrite(GREEN_LED, LOW);  // Turn off Green LED
      delay(250);
      digitalWrite(GREEN_LED, HIGH); // Turn on Green LED (to make it blink)
    }

  // Watchdog counter used to make sure the flow rate stays consistent for 5 cycles
    if (flow < 83.81 || flow > 86.19)
    {
      k = 0;  // Reset Counter to 0
    }

    else if ((flow >= 83.81 && flow <= 86.19) && k < 3)
    {
      k++;  // Increment by 1
    }

  // Flow Verification Loop:
    while ((flow >= 83.81 && flow <= 86.19) && k == 3)
    {
      flow = Read_Sensor("Flow");  // Read the Flow from the TSI 5310 FlowMeter
      digitalWrite(GREEN_LED, HIGH);  // Turn on Green LED
      if (DEBUG) {USBSERIAL.printf("85 SLPM Achieved!: %lf\n", flow);}
      delay(350);
    }
}

// Computes the PID (Porportion, Intregal, Derivative) to determine the most effective ΔSpeed to achieve 85 SLPM
  double Compute_PID(double inp)
  { 
    // Local Variables:
      unsigned long currentTime;
      double kp = 1, kd = 1, elapsedTime, error, rateError, setPoint = 85.0;
      bool debug = false;
   
    // Compute ElapsedTime:
      currentTime = millis();  // Get current time
      elapsedTime = (double)(currentTime - previousTime);  // Compute time elapsed from previous computation
      if (debug) {USBSERIAL.printf("Current Time: %ul\n", currentTime);}
      if (debug) {USBSERIAL.printf("Elapsed Time: %lf\n", elapsedTime);}
  
    // Compute Error & Derivative:
      error = setPoint - inp;  // Determine error
      rateError = (error - previous_error) / elapsedTime;  // Compute derivative
      if (debug) {USBSERIAL.printf("Error: %lf\n", error);}
      if (debug) {USBSERIAL.printf("Derivative: %lf\n", rateError);}
  
    // Compute PID Output:
      double out = (kp * error) + (kd * rateError);  // PID output
      if (debug) {USBSERIAL.printf("PID Output: %lf\n", out);}
  
    // Remember Current Error & Time:
      previous_error = error;  // Remember current error
      previousTime = currentTime;  // Remember current time
      if (debug) {USBSERIAL.printf("Last Error: %lf\n", previous_error);}
      if (debug) {USBSERIAL.printf("Previous Time: %ul\n", previousTime);}
    
    // Return Output:
      return out;  // Have method return the PID output
  }

// Calibrates System to 85 SLPM at 0 mmH2O
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
      if (debug) {USBSERIAL.println("Incorrect Parameter Given To Flow Meter");}
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

// Reboots BLDC Fan Blower's speed back to a hardware safe speed
void Reboot()
{
  bool debug = true;
  
  // Watchdog code to protect the FilterCheck against hardware failure
    if (debug) {USBSERIAL.println("Fan Speed to HIGH!");}
    analogWrite(PWM, 0);  // Change PWM value to new PWM value
    counter = 0;
    
    while (counter <= 30)
    {
      if (debug) {USBSERIAL.println("Inside Reboot Loop!");}
      digitalWrite(BLUE_LED, HIGH);  // Turn off Blue LED
      digitalWrite(GREEN_LED, HIGH);  // Turn off Green LED
      delay(250);
      digitalWrite(BLUE_LED, LOW);  // Turn on Blue LED (to make it blink)
      digitalWrite(GREEN_LED, LOW); // Turn on Green LED (to make it blink)
      counter += 1;
      delay(250);
    }
}
