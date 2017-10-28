#include <Servo.h>
#include <Ultrasonic.h>
#define MINIMAL_REQUIRED_WATER_LEVEL 15
#define SOIL_MOISTURE_THRESHOLD 990
#define SOIL_MOISTURE_HYSTERESIS 10

Servo valveServo;
bool valveOpened = false;
int sensorPin = A0;
const int buttonPin = 2;
int stepNumber = 0;
Ultrasonic ultrasonic(12, 13);
int indicatorPin = 8;

void setup() {
  valveServo.attach(9);
  pinMode(buttonPin, INPUT);
  pinMode(indicatorPin, OUTPUT);
  digitalWrite(indicatorPin, LOW);
  Serial.begin(9600);
}

void loop() {

  int soilMoisture = analogRead(sensorPin);
  int isButtonPressed = digitalRead(buttonPin);
  int waterLevel = ultrasonic.distanceRead();
  // Note that the ultrasonic sensor measures distance from a sensor located on
  // the top of the tank to a water surface, not a real water level.

  switch (stepNumber){
    case 0 :
      // Wait for a one time watering request.
      if(isButtonPressed){
        stepNumber = 1;
      }
      break;
    case 1:
      // Check if there is enough water and wait otherwise.
      if(waterLevel < MINIMAL_REQUIRED_WATER_LEVEL) {
        stepNumber = 2;
      }
      break;
    case 2:
      // Wait for the soil to become dry and start watering.
      if(soilMoisture < SOIL_MOISTURE_THRESHOLD - SOIL_MOISTURE_HYSTERESIS) {
        stepNumber = 3;
        valveOpened = true;
      }
      break;
    case 3:
      // Water the plant until the soil is wet again.
      if(soilMoisture > SOIL_MOISTURE_THRESHOLD + SOIL_MOISTURE_HYSTERESIS){
        stepNumber = 0;
        valveOpened = false;
      }
      break;
  }

  valveServo.write(valveOpened ? 180 : 0);
  // User will be notifed by an indicator that there is not enough water.
  digitalWrite(indicatorPin, stepNumber == 1 ? HIGH : LOW);

  // Debug info
  Serial.println("========== DBG ==========");
  Serial.print("Step number: ");
  Serial.println(stepNumber);
  Serial.print("Water level: ");
  Serial.print(waterLevel);
  Serial.println("cm");
  Serial.print("Raw soil moisture from ADC: ");
  Serial.println(soilMoisture);
  Serial.print("Valve opened: ");
  Serial.print(valveOpened ? "yes" : "no");
  Serial.println("deg");

  // Loop delay
  delay(1000);
}
