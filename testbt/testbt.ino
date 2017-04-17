int led = 9;

void setup() {
  pinMode(led, OUTPUT);  
  Serial.begin(9600);
  Serial.print("AT+NAME=mstanek\r\n");
}

void loop() {
  digitalWrite(led, HIGH);   // turn the LED on (HIGH is the voltage level)
  delay(1000);               // wait for a second
  digitalWrite(led, LOW);    // turn the LED off by making the voltage LOW
  delay(1000);               // wait for a second
}
