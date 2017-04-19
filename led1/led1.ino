#include <Wire.h>
#include <Adafruit_BMP085.h>
#include "TimerOne.h"
//http://ai2.appinventor.mit.edu/
//http://appinventor.mit.edu/test/
//http://mikrokontrolery.blogspot.com/2011/03/android-bluetooth-pierwsza-aplikacja-APP-Inventor-cz-II-v2.html

int btcmd;

int ledpwm;
int ledon;
int ledtim;

int pinled = 6;

Adafruit_BMP085 bmp;

void callback() {
  if(++ledtim % 2) {
    analogWrite(pinled, 0);
    return;
  }
  if(!ledon) {
    analogWrite(pinled, 0);
    return;
  }
  analogWrite(pinled, ledpwm);
}

void setup() {
  Serial.begin(9600);
  //Serial.print("AT+NAME=mstanek\r\n");
  //Serial.print("AT+VERSION?\r\n");
  pinMode(pinled, OUTPUT);
  pinMode(13, OUTPUT);
  Timer1.initialize(500000);
  Timer1.attachInterrupt(callback);
  if (!bmp.begin()) {
    for(;;) {
      digitalWrite(13, !digitalRead(13));
      delay(100);
    }
  }
  btcmd = 0;
  ledpwm=255;
  ledon=1;
  ledtim=0;
}

void loop() {
  
  if(Serial.available() > 0) {
    char b = Serial.read();
    if(b == '.') {
      btcmd = 0;
    } else {
      btcmd++;
    }
    
    switch(btcmd) {
      case 1: {
        // brightness '0'-'9'
        ledpwm = map(b - '0', 0, 9, 0, 255);
        break;
      }
      case 2: {
        // on='1', off='0'
        if(b == '1') {
          ledon = 1;
        } else {
          ledon = 0;
        }
        break;
      }
      case 3: {
        // frequency
        Timer1.setPeriod(map(b - '0', 0, 9, 1000000, 100000));
        break;
      }
    }
  }
  
  // MESSAGE FORMAT
  // <msg type char><value>{n}
  
  int temp = (int)(bmp.readTemperature()*10);
  char temp_sign = (temp < 0)?'-':'+';
  temp = (temp < 0)?-temp:temp;
  int temp_1 = temp % 10;
  int temp_10 = (temp-temp_1)/10 % 10;
  int temp_100 = (temp-temp_10-temp_1)/100 % 10; 
  Serial.print('T');
  Serial.print(temp_sign);
  Serial.print(temp_100);
  Serial.print(temp_10);
  Serial.print(temp_1);
  Serial.print('\n');
  
  int p = ((long)(bmp.readPressure()) - 90000) / 100;
  int p_1 = p % 10;
  int p_10 = (p-p_1)/10 % 10;
  int p_100 = (p-p_10-p_1)/100 % 10;
  Serial.print('P');
  Serial.print(p_100);
  Serial.print(p_10);
  Serial.print(p_1);
  Serial.print('\n');
}

