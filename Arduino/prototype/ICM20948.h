#include <Adafruit_ICM20948.h>
#include <Adafruit_ICM20X.h>
Adafruit_ICM20948 icm;

#define ICM_CS 10
// For software-SPI mode we need SCK/MOSI/MISO pins
#define ICM_SCK 13
#define ICM_MISO 12
#define ICM_MOSI 11

bool init_sensors(void) {

  // Try to initialize!
  if (!icm.begin_I2C()) {
    if (!icm.begin_SPI(ICM_CS)) {
      if (!icm.begin_SPI(ICM_CS, ICM_SCK, ICM_MISO, ICM_MOSI)) {

        //Serial.println("Failed to find ICM20948 chip");
        Serial.write("Failed to find ICM20948 chip");
        while (1) {
          delay(10);
        }
      }
    }
  }
  accelerometer = icm.getAccelerometerSensor();
  gyroscope = icm.getGyroSensor();
  magnetometer = icm.getMagnetometerSensor();

  return true;
}

void setup_sensors(void) {
  // set lowest range
  icm.setAccelRange(ICM20948_ACCEL_RANGE_4_G);
  icm.setGyroRange(ICM20948_GYRO_RANGE_250_DPS);
  icm.setMagDataRate(AK09916_MAG_DATARATE_100_HZ);
}
