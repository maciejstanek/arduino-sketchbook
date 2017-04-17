#include <Servo.h>
#include <Stepper.h>

int pinButton = 2;

int pinStepA = 5;
int pinStepB = 6;
int pinStepC = 10;
int pinStepD = 11;
int stepsPerRev = 20;
Stepper stepper(stepsPerRev, pinStepA, pinStepB, pinStepC, pinStepD);
int commonDelay = 10;

int pinSelX = 12;
int pinSelY = 13;

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
int pinManualOverride = 4;

int pinZeroX = 7;
int pinZeroY = 8;

void resetOrigin() {
  Serial.println("Resetting origin...");
  Serial.println("Doing Y axis...");
  digitalWrite(pinSelX, 0);
  digitalWrite(pinSelY, 1);
  delay(commonDelay);
  while(!digitalRead(pinZeroY)) {
    stepper.step(-1);
    delay(commonDelay);
  }
  currentY = 0;
  Serial.println("Y axis done");
  Serial.println("Doing X axis...");
  digitalWrite(pinSelX, 1);
  digitalWrite(pinSelY, 0);
  delay(commonDelay);
  while(!digitalRead(pinZeroX)) {
    stepper.step(-1);
    delay(commonDelay);
  }
  currentX = 0;
  Serial.println("X axis done");
  Serial.println("Resetting origin finished");
}

void setup() {
  pinMode(pinZeroX, INPUT);
  pinMode(pinZeroY, INPUT);
  pinMode(pinButton, INPUT);
  pinMode(pinManualOverride, INPUT);
  pinMode(pinSelX, OUTPUT);
  pinMode(pinSelY, OUTPUT);
  digitalWrite(pinSelX, 0);
  digitalWrite(pinSelY, 0);
  servo.attach(pinServo);
  servo.write(servoHigh);
  stepper.setSpeed(100);
  Serial.begin(115200);
  Serial.println("\n");
  resetOrigin();
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
  manual = digitalRead(pinManualOverride) ? manual : 1;
  
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
  float propA = (float)(valJoystickX + 1) / (float)(valJoystickY + 1); // +1 to prevent from div0 error
  float propB = (float)(valJoystickX + 1) / (float)(1024 - valJoystickY);
  int dirX = 0;
  int dirY = 0;
  if(manual) {
    if(!(0.9 < propA && propA < 1.1 && 0.9 < propB && propB < 1.1)) {
      dirX = propA > 1 && propB > 1 ? -1 : (propA < 1 && propB < 1 ? 1 : 0);
      dirY = propA > 1 && propB < 1 ? -1 : (propA < 1 && propB > 1 ? 1 : 0);
    }
  } else {
    dirX = autoDirX;
    dirY = autoDirY;
  }
  
  servo.write(manual ? servoAngle : autoServoAngle);

  if(dirY) {
    digitalWrite(pinSelX, 0);
    digitalWrite(pinSelY, 1);
    delay(commonDelay);
    currentY += dirY;
    stepper.step(dirY);
    delay(commonDelay); 
  } else if(dirX) {
    digitalWrite(pinSelX, 1);
    digitalWrite(pinSelY, 0);
    delay(commonDelay);
    currentX += dirX;
    stepper.step(dirX);
    delay(commonDelay); 
  } else {
    delay(2 * commonDelay);
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
  
}
