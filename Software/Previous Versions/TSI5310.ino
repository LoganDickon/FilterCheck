char s;
String c;
String buf[5];
String i;
int k = 0;
String d;
float f;
double total;

void setup() {
  // put your setup code here, to run once:
  pinMode(8, OUTPUT);
  Serial.begin(9600);
  while (!Serial && (millis() < 5000)) {};
  Serial1.begin(115200);
  delay(1000);
  digitalWrite(8, HIGH);
  Serial.println("Start Timer!");
  delay(25000);
  Serial1.print("?\r\n");
  Serial1.print("SUS\r\n");
  Serial1.print("SG0\r\n");
  Serial1.print("SSR0005\r\n");
  Serial1.print("LPZ\r\n");
  Serial1.print("DAFxxxxx0005\r\n");

  delay(1000);
  
  while (Serial1.available()){
    s = Serial1.read();
    c += s;
  }
  c.replace("OK","");
  c.replace("\r","");
  c.replace("\n","");
  Serial.println("OUTPUT: " + c);

  // Convert String Into String Array
  c = c + ","; // Add a ',' to the end of the string
  for (int j = 0; j < c.length(); j++) {
    if ((c[j]) != ',') {
      i += c[j]; 
    }
    if ((c[j]) == ',') {
      buf[k] = i;
      i = "";
      k++;
    }
  }
  for (int p = 0; p < 5; p++){
    d = buf[p];
    f = d.toFloat();
    total += f;
  }
  total /= 5;
  Serial.printf("total: %lf\n", total);
  
  Serial.println("Done!");
}

void loop() {
  // put your main code here, to run repeatedly:
}
