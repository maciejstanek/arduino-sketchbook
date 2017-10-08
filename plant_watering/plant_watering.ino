#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 externalDisplay(OLED_RESET);

byte analogInPin = A0;
int sensorValue = 0;

#define FILTER_SAMPLES 6
int sensorSamplesQueue[FILTER_SAMPLES];
int sensorAverage = 0;

void setupExternalDisplay() {
  externalDisplay.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  externalDisplay.clearDisplay();
  externalDisplay.display();
}

void readSensorValue() {
  delay(100); // Make sure the voltage level is more or less stable
  sensorValue = analogRead(analogInPin);
}

void insertSensorValueToSamplesQueue() {
  for(byte i = FILTER_SAMPLES - 1; i > 0; i--) {
    sensorSamplesQueue[i] = sensorSamplesQueue[i - 1];
  }
  sensorSamplesQueue[0] = sensorValue;
}

void fillSamplesQueueWithCurrentSensorValue() {
  for(byte i = 0; i < FILTER_SAMPLES; i++) {
    sensorSamplesQueue[i] = sensorValue;
  }
}

void calculateAverageSensorValue() {
  sensorAverage = 0;
  for(byte i = 0; i < FILTER_SAMPLES; i++) {
    sensorAverage += sensorSamplesQueue[i];
  }
  sensorAverage /= FILTER_SAMPLES;
}

void displayDebug() {
  printSerialDebug();
  displayExternalDisplayDebug();
}

void printSerialDebug() {
  Serial.print("raw moisture = ");
  Serial.println(sensorValue);
}

void displayExternalDisplayDebug() {
  externalDisplay.clearDisplay();
  externalDisplay.setTextSize(1);
  externalDisplay.setTextColor(WHITE);
  externalDisplay.setCursor(0, 0);
  externalDisplay.print("Raw moisture from ADC");
  byte LineIntersectionPointX = 56;
  byte LineIntersectionPointY = 12;
  externalDisplay.drawLine(0, LineIntersectionPointY - 1, externalDisplay.width() - 1, LineIntersectionPointY - 1, WHITE);
  externalDisplay.drawLine(0, LineIntersectionPointY + 1, LineIntersectionPointX - 1, LineIntersectionPointY + 1, WHITE);
  externalDisplay.drawLine(LineIntersectionPointX + 1, LineIntersectionPointY + 1, externalDisplay.width() - 1, LineIntersectionPointY + 1, WHITE);

  externalDisplay.setCursor(0, 16);
  for(byte i = 0; i < FILTER_SAMPLES; i++) {
    externalDisplay.print("s[");
    externalDisplay.print(i);
    externalDisplay.print("]=");
    externalDisplay.println(sensorSamplesQueue[i]);
  }
  externalDisplay.drawLine(LineIntersectionPointX - 1, LineIntersectionPointY + 1, LineIntersectionPointX - 1, externalDisplay.height() - 1, WHITE);
  externalDisplay.drawLine(LineIntersectionPointX + 1, LineIntersectionPointY + 1, LineIntersectionPointX + 1, externalDisplay.height() - 1, WHITE);

  externalDisplay.setCursor(72, 24);
  externalDisplay.println("Average");
  externalDisplay.setCursor(72, 40);
  externalDisplay.setTextSize(2);
  externalDisplay.println(sensorAverage);
  externalDisplay.display();
}

void setup() {
  Serial.begin(9600);
  setupExternalDisplay();
  readSensorValue();
  fillSamplesQueueWithCurrentSensorValue();
  calculateAverageSensorValue();
  displayDebug();
}

void loop() {
  delay(1000); // Main loop delay
  readSensorValue();
  insertSensorValueToSamplesQueue();
  calculateAverageSensorValue();
  displayDebug();
}
