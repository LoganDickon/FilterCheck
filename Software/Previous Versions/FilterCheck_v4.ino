// Created By: Logan Dickon
// Company: Atorlabs
// Date Created: 3/09/2022
// Date Modified: 3/14/2022
// This Program  

// Pre-Cursor Directives:
  #include <ZzzMovingAvg.h>

// Device Constants:
  #define PWM         2           // PWM Signal from pin 2 on the Teensy 2.5
  #define RELAY       8           // Relay i/o pin from pin 8 on the Teensy 3.5
  #define usbSerial   Serial      // Rename Serial Main to usbSerial
  #define FlowMeter   Serial1     // Rename Serial1 to FlowMeter
  #define DEBUG       true        // <<--- with this set to true a lot of serial screen output will be seen
                                  // an easy way to debug progress;  

// Global Variables:
  double Flow;
  double Flow_Avg = 0;
  int fan_speed = 25;
  int i = 0;
  bool Loop = true;

// Class Contructor:
  ZzzMovingAvg <5, float, float> Flow_Avger;
 
void setup()
{   
  // Pin Initialization:
    pinMode(PWM, OUTPUT);  // Set PWM pin to OUTPUT mode
    pinMode(RELAY, OUTPUT);  // Set RELAY pin to OUTPUT mode

  // Serial Initialization:
    usbSerial.begin(9600);  // Start serial for Teensy to PC
    delay(1000);
    FlowMeter.begin(115200);  // Start serial for Teensy to FlowMeter
    delay(1000);

  // Turn On FlowMeter & Fan Blower:
    digitalWrite(PWM, 15);  // Turn On Fan Blower
    digitalWrite(RELAY, HIGH);  // Turn Relay HIGH to allow power to the FlowMeter
    delay(26000);

  // FlowMeter Startup ASCII Commands:
    FlowMeter.print("SUS\r\n");  // Sets flow measurement to Standard Flow
    FlowMeter.print("SG0\r\n");  // Set gas calibration to Air
    FlowMeter.print("SSR0005\r\n");  // Set sample rate to 5
    delay(1000);

  // Set LPM to 85 LPM:
    while (i <= 49)
    {
      Flow = Read_Flow();
      usbSerial.println(Flow);
      Flow_Avg += Flow;
      i++;
    }
    Flow_Avg = Flow_Avg / 50;
    usbSerial.printf("Flow Avg: %lf\n", Flow_Avg);
}

void loop() 
{
  // put your main code here, to run repeatedly:
    /*Flow = Read_Flow();
    while (((Flow < 83.81) || (Flow > 86.19)) && i <= 5)
    {
      if (Flow < 83.81)
      {
        fan_speed += 1;
        analogWrite(PWM, fan_speed);
        Flow = Read_Flow();
        if (DEBUG) {usbSerial.printf("Flow: %lf LPM\n", Flow);}
        if (DEBUG) {usbSerial.printf("FanSpeed: %d \n", fan_speed);}
        if (DEBUG) {usbSerial.printf("i: %d\n", i);}
        delay(500);
      }
      if (Flow > 86.19)
      {
        fan_speed -= 1;
        analogWrite(PWM, fan_speed);
        Flow = Read_Flow();
        if (DEBUG) {usbSerial.printf("Flow: %lf LPM\n", Flow);}
        if (DEBUG) {usbSerial.printf("FanSpeed: %d \n", fan_speed);}
        if (DEBUG) {usbSerial.printf("i: %d\n", i);}
        delay(500);
      }
    }
    while (Flow >= 83.81 && Flow <= 86.19)
    {
      Flow = Read_Flow();
      i++;
      if (DEBUG) {usbSerial.printf("i: %d\n", i);}
      if (Flow < 83.81 || Flow > 86.19)
      {
        i = 1;
        if (DEBUG) {usbSerial.printf("Flow Went Out Of Range: %lf LPM\n", Flow);}
      }
    }
    if (i == 5)
    {
      i = 1;
      if (DEBUG) {usbSerial.println("85 LPM Achieved!");}
    }*/
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
        total_char += serial_buf_String[i];  // Add each char to a stirng until a ',' is read
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

  // Get Average Flow & Return Flow
    avg = total / 5;  // Divide total by the number of array indexes
    return avg; // Return Flow Data
    
}
