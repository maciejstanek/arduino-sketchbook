int pin_PIR = 2;

void setup() {                
  Serial.begin(9600);
  pinMode(pin_PIR, INPUT);     
}

void loop() {
  if(digitalRead(pin_PIR)) {
    Serial.println("ON");
  } else {
    Serial.println("OFF");
  }
  delay(1000);
}
