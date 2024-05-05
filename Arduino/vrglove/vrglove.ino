#include <Uduino_Wifi.h>
#include <Adafruit_Sensor_Calibration.h>
#include <Adafruit_AHRS.h>
#include <driver/adc.h>

Adafruit_Sensor *accelerometer, *gyroscope, *magnetometer;

#include "ICM20948.h"

Uduino_Wifi uduino("uduinoBoard"); // Declare and name your object

// pick your filter! slower == better quality output
Adafruit_NXPSensorFusion filter; // slowest
//Adafruit_Madgwick filter;  // faster than NXP
//Adafruit_Mahony filter;  // fastest/smalleset

#if defined(ADAFRUIT_SENSOR_CALIBRATION_USE_EEPROM)
  Adafruit_Sensor_Calibration_EEPROM cal;
#else
  Adafruit_Sensor_Calibration_SDFat cal;
#endif

#define FILTER_UPDATE_RATE_HZ 100
#define PRINT_EVERY_N_UPDATES 1

uint32_t timestamp;

void setup()
{
  Serial.begin(115200);
#if defined (__AVR_ATmega32U4__) // Leonardo
  while (!Serial) {}
#elif defined(__PIC32MX__)
  delay(1000);
#endif

  // Optional functions,  to add BEFORE connectWifi(...)
  uduino.setPort(4222);   // default 4222
  uduino.connectWifi("SSID", "PASSWORD");

  uduino.addCommand("s", SetMode);
  uduino.addCommand("d", WritePinDigital);
  uduino.addCommand("a", WritePinAnalog);
  uduino.addCommand("rd", ReadDigitalPin);
  uduino.addCommand("r", ReadAnalogPin);
  uduino.addCommand("br", BundleReadPin);
  uduino.addCommand("b", ReadBundle);

  if (!cal.begin()) {
    Serial.println("Failed to initialize calibration helper");
  } else if (! cal.loadCalibration()) {
    Serial.println("No calibration loaded/found");
  }

  if (!init_sensors()) {
    Serial.println("Failed to find sensors");
    while (1) delay(10);
  }
  
  accelerometer->printSensorDetails();
  gyroscope->printSensorDetails();
  magnetometer->printSensorDetails();

  setup_sensors();
  filter.begin(FILTER_UPDATE_RATE_HZ);
  timestamp = millis();

  Wire1.setClock(400000); // 400KHz

  pinMode(MOSI, OUTPUT);  // S0
  pinMode(MISO, OUTPUT);  // S1
  pinMode(SCK, OUTPUT);  // S2
  pinMode(RX, OUTPUT);   // S3
  pinMode(TX, INPUT_PULLDOWN); // SIG

  digitalWrite(MOSI, LOW);
  digitalWrite(MISO, LOW);
  digitalWrite(SCK, LOW);
  digitalWrite(RX, LOW);

  analogSetAttenuation(ADC_11db);
}

void loop()
{
  uduino.update();
  if(uduino.isConnected()) {
    float roll, pitch, heading;
    float gx, gy, gz;
    static uint8_t counter = 0;

    if ((millis() - timestamp) < (1000 / FILTER_UPDATE_RATE_HZ)) {
      return;
    }
    timestamp = millis();
    // Read the motion sensors
    sensors_event_t accel, gyro, mag;
    accelerometer->getEvent(&accel);
    gyroscope->getEvent(&gyro);
    magnetometer->getEvent(&mag);

    cal.calibrate(mag);
    cal.calibrate(accel);
    cal.calibrate(gyro);
    // Gyroscope needs to be converted from Rad/s to Degree/s
    // the rest are not unit-important
    gx = gyro.gyro.x * SENSORS_RADS_TO_DPS;
    gy = gyro.gyro.y * SENSORS_RADS_TO_DPS;
    gz = gyro.gyro.z * SENSORS_RADS_TO_DPS;

    // Update the SensorFusion filter
    filter.update(gx, gy, gz, 
                  accel.acceleration.x, accel.acceleration.y, accel.acceleration.z, 
                  mag.magnetic.x, mag.magnetic.y, mag.magnetic.z);

    // only print the calculated output once in a while
    if (counter++ <= PRINT_EVERY_N_UPDATES) {
      return;
    }
    // reset the counter
    counter = 0;

#if defined(AHRS_DEBUG_OUTPUT)
    Serial.print("Raw: ");
    Serial.print(accel.acceleration.x, 4); Serial.print(", ");
    Serial.print(accel.acceleration.y, 4); Serial.print(", ");
    Serial.print(accel.acceleration.z, 4); Serial.print(", ");
    Serial.print(gx, 4); Serial.print(", ");
    Serial.print(gy, 4); Serial.print(", ");
    Serial.print(gz, 4); Serial.print(", ");
    Serial.print(mag.magnetic.x, 4); Serial.print(", ");
    Serial.print(mag.magnetic.y, 4); Serial.print(", ");
    Serial.print(mag.magnetic.z, 4); Serial.println("");
#endif

    // print the heading, pitch and roll
    roll = filter.getRoll();
    pitch = filter.getPitch();
    heading = filter.getYaw();

    uduino.print(roll);
    uduino.print(",");
    uduino.print(pitch);
    uduino.print(",");
    uduino.print(heading);
    uduino.print(",");
    
    /// read flex sensors
    uduino.print(readFlexSensor(LOW, LOW, LOW, LOW)); // index
    uduino.print(",");
    uduino.print(readFlexSensor(HIGH, LOW, LOW, LOW));  // index knuckle
    uduino.print(",");
    uduino.print(readFlexSensor(LOW, HIGH, LOW, LOW));  // middle
    uduino.print(",");
    uduino.print(readFlexSensor(HIGH, HIGH, LOW, LOW));   // middle knuckle
    uduino.print(",");
    uduino.print(readFlexSensor(LOW, LOW, HIGH, LOW));  // ring
    uduino.print(",");
    uduino.print(readFlexSensor(HIGH, LOW, HIGH, LOW));   // ring knuckle
    uduino.print(",");
    uduino.print(readFlexSensor(LOW, HIGH, HIGH, LOW));   // pinky
    uduino.print(",");
    uduino.print(readFlexSensor(HIGH, HIGH, HIGH, LOW));    // pinky knuckle
    uduino.print(",");
    uduino.print(readFlexSensor(LOW, LOW, LOW, HIGH));  // thumb
    uduino.print(",");
    uduino.print(readFlexSensor(HIGH, LOW, LOW, HIGH));   // thumb knuckle
    uduino.print(",");
    uduino.println(readFlexSensor(LOW, HIGH, LOW, HIGH));   // thumb adduction
  }
}

float readFlexSensor(uint8_t S0, uint8_t S1, uint8_t S2, uint8_t S3) {
    int num_avg = 16;

    digitalWrite(MOSI, S0);
    digitalWrite(MISO, S1);
    digitalWrite(SCK, S2);
    digitalWrite(RX, S3);

    uint16_t val = analogRead(TX);

    for (int i = 0; i < num_avg - 1; i++) {
      val += analogRead(TX);
    }

    digitalWrite(MOSI, LOW);
    digitalWrite(MISO, LOW);
    digitalWrite(SCK, LOW);
    digitalWrite(RX, LOW);

    delay(1);

    float avg = (float)(val / (num_avg - 1));

    return avg / 100;
}