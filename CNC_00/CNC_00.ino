#include <Servo.h>
#include <Stepper.h>

int pinButton = 2;

// Stepper motors for XY axis
int stepsPerRev = 20;
Stepper stepperY(stepsPerRev, 5, 6, 4, 7);
Stepper stepperX(stepsPerRev, 8, 9, 10, 11);

// Servo for raising/lowering tool
int pinServo = 3;
int pinServoKnob = A0;
//int servoHigh = 110;
//int servoLow = 150;
int servoHigh = 110;
int servoLow = 100;
int servoTune = 10;
Servo servo;

// Status data
int currentX = 0;
int currentY = 0;
int dirX = 0;
int dirY = 0;
int zx = 0;
int zy = 0;
boolean manual = true;
char cmd = '-';
int status = 0;

int pinJoystickX = A5;
int pinJoystickY = A4;
int autoServoAngle = servoHigh;
int servoAngle = servoHigh;
int modeStatusLED = 12;
int pinManualOverride = 17; // A3 as a digital pin
int pinZeroX = 15; // A1 as a digital pin
int pinZeroY = 16; // A2 as a digital pin

void printStatusJSON() {
  Serial.print("{\"c\":{\"x\":");
  Serial.print(currentX);
  Serial.print(",\"y\":");
  Serial.print(currentY);
  Serial.print("},\"d\":{\"x\":");
  Serial.print(dirX);
  Serial.print(",\"y\":");
  Serial.print(dirY);
  Serial.print("},\"z\":{\"x\":");
  Serial.print(zx);
  Serial.print(",\"y\":");
  Serial.print(zy);
  Serial.print("},\"man\":");
  Serial.print(manual);
  Serial.print(",\"cmd\":\"");
  Serial.print(cmd);
  Serial.print("\",\"sta\":");
  Serial.print(status);
  Serial.print("}\n");
}

void resetOrigin() {
	status = 1;
	printStatusJSON();
  while(!digitalRead(pinZeroY)) {
    stepperY.step(-1);
  }
  currentY = 0;
	status = 2;
	printStatusJSON();
  while(!digitalRead(pinZeroX)) {
    stepperX.step(-1);
  }
  currentX = 0;
	status = 0;
	printStatusJSON();
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
	status = 0;
  
  int knobTuning = map(analogRead(pinServoKnob), 0, 1023, -servoTune, servoTune);
  servoAngle = digitalRead(pinButton) ? servoHigh : servoLow + knobTuning;

  cmd = '-';
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
      resetOrigin(); // It's a blocking function, consider moving it outside this logic
      break;
  }

  int valJoystickX = analogRead(pinJoystickX);
  int valJoystickY = analogRead(pinJoystickY);
  if(manual) {
    dirX = valJoystickX < 447 ? 1 : (valJoystickX > 575 ? -1 : 0);
    dirY = valJoystickY < 447 ? -1 : (valJoystickY > 575 ? 1 : 0);
  } else {
    dirX = autoDirX;
    dirY = autoDirY;
  }
  
  servo.write(manual ? servoAngle : autoServoAngle);
  
  zx = digitalRead(pinZeroX);
  zy = digitalRead(pinZeroY);
  
  if(zx && dirX < 0) {
    dirX = 0;
  }
  if(zy && dirY < 0) {
    dirY = 0;
  }

  if(dirY) {
    currentY += dirY;
    stepperY.step(dirY);
  }
  if(dirX) {
    currentX += dirX;
    stepperX.step(dirX);
  }

	printStatusJSON(); 
  delay(10);
  
}
