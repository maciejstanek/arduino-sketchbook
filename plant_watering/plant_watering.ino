#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 4
Adafruit_SSD1306 externalDisplay(OLED_RESET);

byte analogInPin = A0;
int sensorValue = 0;

#define FILTER_SAMPLES 6
int sensorSamples[FILTER_SAMPLES];
int sensorAverage = 0;

void setup() {
  Serial.begin(9600);
  externalDisplay.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  externalDisplay.clearDisplay();
  externalDisplay.display();
  for(byte i = 0; i < FILTER_SAMPLES; i++) {
    sensorSamples[i] = 0;
  }
}

void readSensorValue() {
  delay(100); // Make sure the voltage level is more or less stable
  sensorValue = analogRead(analogInPin);
  for(byte i = FILTER_SAMPLES - 1; i > 0; i--) {
    sensorSamples[i] = sensorSamples[i - 1];
  }
  sensorSamples[0] = sensorValue;
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
  externalDisplay.drawLine(0, 12, externalDisplay.width() - 1, 12, WHITE);

  externalDisplay.setCursor(0, 16);
  for(byte i = 0; i < FILTER_SAMPLES; i++) {
    externalDisplay.print("s[");
    externalDisplay.print(i);
    externalDisplay.print("]=");
    externalDisplay.println(sensorSamples[i]);
  }
  externalDisplay.drawLine(56, 12, 56, externalDisplay.height() - 1, WHITE);

  externalDisplay.setCursor(64, 16);
  externalDisplay.println("Average:");
  externalDisplay.setCursor(64, 32);
  externalDisplay.setTextSize(2);
  externalDisplay.println(sensorAverage);
  externalDisplay.display();
}

void calculateAverageSensorValue() {
  sensorAverage = 0;
  for(byte i = 0; i < FILTER_SAMPLES; i++) {
    sensorAverage += sensorSamples[i];
  }
  sensorAverage /= FILTER_SAMPLES;
}

void loop() {
  readSensorValue();
  calculateAverageSensorValue();
  displayDebug();
  delay(1000);
}
