void setup() {
  pinMode(13, OUTPUT);
  Serial.begin(9600);
  Serial.println("Test");
}

void loop() {
  if (Serial.available() > 0) {
    byte c = Serial.read();
    switch(c) {
      case '0':
        digitalWrite(13, 0);
        Serial.println("off");
        break;
      case '1':
        digitalWrite(13, 1);
        Serial.println("on");
        break;        
    }
  }
}

