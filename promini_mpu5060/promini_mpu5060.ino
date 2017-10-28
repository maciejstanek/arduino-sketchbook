#include <Wire.h>
#include <TimerOne.h>
#define VALUES_COUNT 7

const int MPUAddress = 0x68;
const int MPURegisterData = 0x3B; // ACCEL_XOUT_H register
const int MPURegisterPower = 0x6B; // PWR_MGMT_1 register
volatile uint32_t tick = 0;
volatile uint32_t lastProcessedTick = 0;
// 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)     
// 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
// 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
// 0x41 (TEMP_OUT_H)   & 0x42 (TEMP_OUT_L)
// 0x43 (GYRO_XOUT_H)  & 0x44 (GYRO_XOUT_L)
// 0x45 (GYRO_YOUT_H)  & 0x46 (GYRO_YOUT_L)
// 0x47 (GYRO_ZOUT_H)  & 0x48 (GYRO_ZOUT_L)

void timerTick() {
  tick++;
}

void setup() {
  Wire.begin();
  Wire.beginTransmission(MPUAddress);
  Wire.write(MPURegisterPower); // PWR_MGMT_1 register
  Wire.write(0); // set to zero (wakes up the MPU-6050)
  Wire.endTransmission(true);

  Serial.begin(1000000);

  Timer1.initialize(10000);
  Timer1.attachInterrupt(timerTick);
}

void loop() {
  if(lastProcessedTick < tick) {
    lastProcessedTick = tick;

    Wire.beginTransmission(MPUAddress);
    Wire.write(MPURegisterData);
    Wire.endTransmission(false);
    Wire.requestFrom(MPUAddress, 14, true); // Request a total of 14 registers.

    int16_t rawValue[VALUES_COUNT];
    for(int i = 0; i < VALUES_COUNT; i++) {
      rawValue[i] = Wire.read() << 8 | Wire.read();
    }

    char buffer[10];
    sprintf(buffer, "%08x,", tick);
    Serial.print(buffer);
    for(int i = 0; i < VALUES_COUNT; i++) {
      sprintf(buffer, "%04x,", rawValue[i]);
      Serial.print(buffer);
    }
    Serial.println("");
  }
}
