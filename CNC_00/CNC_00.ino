#include <Servo.h>
#include <Stepper.h>

int pinButton = 2;

int stepsPerRev = 20;
Stepper stepperY(stepsPerRev, 5, 6, 4, 7);
Stepper stepperX(stepsPerRev, 8, 9, 10, 11);

int pinServo = 3;
int pinServoKnob = A0;
int servoHigh = 110;
int servoLow = 150;
Servo servo;

int pinJoystickX = A5;
int pinJoystickY = A4;

int currentX = 0;
int currentY = 0;
int targetX = 0;
int targetY = 0;
int autoServoAngle = servoHigh;
int servoAngle = servoHigh;
boolean manual = true;
int modeStatusLED = 12;
int pinManualOverride = 17; // A3 as a digital pin

int pinZeroX = 15; // A1 as a digital pin
int pinZeroY = 16; // A2 as a digital pin

void resetOrigin() {
  Serial.println("Resetting origin...");
  Serial.println("Doing Y axis...");
  while(!digitalRead(pinZeroY)) {
    stepperY.step(-1);
  }
  currentY = 0;
  Serial.println("Y axis done");
  Serial.println("Doing X axis...");
  while(!digitalRead(pinZeroX)) {
    stepperX.step(-1);
  }
  currentX = 0;
  Serial.println("X axis done");
  Serial.println("Resetting origin finished");
}

void setup() {
  pinMode(modeStatusLED, OUTPUT);
  pinMode(pinZeroX, INPUT);
  pinMode(pinZeroY, INPUT);
  pinMode(pinButton, INPUT);
  pinMode(pinManualOverride, INPUT);
  servo.attach(pinServo);
  servo.write(servoHigh);
  stepperX.setSpeed(100);
  stepperY.setSpeed(100);
  Serial.begin(115200);
  Serial.println("\n");
  //resetOrigin();
}

void loop() {
  
  int knobTuning = map(analogRead(pinServoKnob), 0, 1023, -20, 20);
  servoAngle = digitalRead(pinButton) ? servoHigh : servoLow + knobTuning;

  char cmd = 0;
  int autoDirX = 0;
  int autoDirY = 0;
  if(Serial.available() > 0) {
    cmd = Serial.read();
  }
  boolean manualOverride = digitalRead(pinManualOverride);
  manual = manualOverride ? manual : 1;
  digitalWrite(modeStatusLED, manual);
  
  switch(cmd) {
    case 'M': // manual
      manual = true;
      autoServoAngle = servoHigh;
      break;
    case 'A': // automatic
      manual = false;
      break;
    case 'N': // north
      autoDirY = 1;
      break;
    case 'S': // south
      autoDirY = -1;
      break;
    case 'E': // east
      autoDirX = 1;
      break;
    case 'W': // west
      autoDirX = -1;
      break;
    case 'U': // marker up
      autoServoAngle = servoHigh;
      break;
    case 'D': // marker down
      autoServoAngle = servoLow + knobTuning;
      break;
    case 'R': // reset origin
      resetOrigin();
      break;
  }

  int valJoystickX = analogRead(pinJoystickX);
  int valJoystickY = analogRead(pinJoystickY);
  int dirX = 0;
  int dirY = 0;
  if(manual) {
    dirX = valJoystickX < 447 ? 1 : (valJoystickX > 575 ? -1 : 0);
    dirY = valJoystickY < 447 ? -1 : (valJoystickY > 575 ? 1 : 0);
  } else {
    dirX = autoDirX;
    dirY = autoDirY;
  }
  
  servo.write(manual ? servoAngle : autoServoAngle);

  if(dirY) {
    currentY += dirY;
    stepperY.step(dirY);
  }
  if(dirX) {
    currentX += dirX;
    stepperX.step(dirX);
  }
  
  Serial.print("{zX=");
  Serial.print(digitalRead(pinZeroX));
  Serial.print("} {zY=");
  Serial.print(digitalRead(pinZeroY));
  Serial.print("} ");
  
  Serial.print("[");
  Serial.print(dirX);
  Serial.print(",");
  Serial.print(dirY);
  Serial.print("] (");
  Serial.print(currentX);
  Serial.print(",");
  Serial.print(currentY);
  Serial.print(")->(");
  Serial.print(targetX);
  Serial.print(",");
  Serial.print(targetY);
  Serial.print(") manual=");
  Serial.print(manual);
  Serial.print(" cmd='");
  Serial.print(cmd);
  Serial.println("'");
  
  delay(10);
  
}
