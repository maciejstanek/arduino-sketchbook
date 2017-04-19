#include "TimerOne.h"
int ledPin=6;
long freq = 500;
int b = 1000;
boolean overwrite = false;
String readString;
int count = 0;
void callback()
{
   count++;
  if(overwrite){
    analogWrite(ledPin, 0);
  }
  else{
    if(count%2 ==0){
    analogWrite(ledPin, b);
    }
    else{analogWrite(ledPin, 0);}
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(ledPin, OUTPUT);
  Timer1.initialize(500000);         // initialize timer1, and set a 1/2 second period
  //Timer1.pwm(9, 512);                // setup pwm on pin 9, 50% duty cycle
  Timer1.attachInterrupt(callback);  // attaches callback() as a timer overflow interrupt
  Serial.println("AT+NAME=makaron\r\n");
}


void loop() {
  while (Serial.available())
  {
    delay(5);
    char c = Serial.read();
    readString += c;
  }
  if(readString.length() > 0)
  {
    Serial.println(readString);
    if(readString=="ON")
    {
      overwrite = false;
      //digitalWrite(ledPin, HIGH);
    }
    else if(readString=="OFF")
    {
      overwrite = true;
      //digitalWrite(ledPin, LOW);
    }
    else if (readString.toInt()>=0 && readString.toInt()<=20)
    {
      b = readString.toInt()*5;
      //analogWrite(ledPin, b);
    }
    else if (readString.toInt()>=30 && readString.toInt()<=40){
      Serial.println(readString.toInt());
      freq =(readString.toInt() - 29)*50;
      Serial.println(freq);
      Serial.println(freq*1000);
      Timer1.setPeriod(freq*1000);
      }

    readString="";
  }
}
