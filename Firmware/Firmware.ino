// Workshop name: "IoT Essentials: Crafting simple but powerful data pipelines"
// Location: SpliTech IoT day 2024, FESB, Split, Croatia
// Date: 25. june 2024.
// Authors:
//   Milovan Medojević <milovan.medojevic@ivi.ac.rs>
//   Dušan Simić <dusan.simic@dmi.uns.ac.rs>
//
// Workshop is based on the paper published at the SplitTech 2024 conference.
// Paper name: "Development of an IoT-Based Real-Time Psychrometric Data
//   Acquisition and Visualization System"
// Authors:
//   Milovan Medojević <milovan.medojevic@ivi.ac.rs>
//   Dušan Simić <dusan.simic@dmi.uns.ac.rs>
//   Milana Medojević <medojevicm@uns.ac.rs>

#include <Wire.h>
#include "Adafruit_BMP280.h"
#include "AHT10.h"
#include <WiFi.h>
#include <WiFiMulti.h>
#include <InfluxDbClient.h>
#include "config.h"


Adafruit_BMP280 bmp;
AHT10 AHT20(AHT10_ADDRESS_0X38, AHT20_SENSOR);

WiFiMulti wifiMulti;

InfluxDBClient client(INFLUXDB_URL, INFLUXDB_NAME);
Point pointDevice("sensors");
Point timesPointDevice("times");

unsigned long loop_start;
unsigned long start_reading;
unsigned long start_sending;
unsigned long iteration_complete;
unsigned long loop_end;
unsigned long iteration_count;

void setup() {
  Serial.begin(115200);

  while (!Serial);

  WiFi.begin();
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI1_SSID, WIFI1_PASS);

  client.setConnectionParamsV1(INFLUXDB_URL, INFLUXDB_NAME, INFLUXDB_USER, INFLUXDB_PASS);

  pointDevice.addTag("device", INFLUXDB_TAG_DEVICE);
  pointDevice.addTag("senslist", INFLUXDB_TAG_SENSOR_LIST);
  timesPointDevice.addTag("ssid", WIFI1_SSID);

  if (client.validateConnection()) {
    Serial.print("Connected to InfluxDB: ");
    Serial.println(client.getServerUrl());
  } else {
    Serial.print("InfluxDB connection failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  if (!bmp.begin()) {
    Serial.println("BMP280 initialization failed...");
    while(1);
  }

  if (!AHT20.begin()) {
    Serial.println("AHT20 initialization failed...");
    while(1);
  }

  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,
                  Adafruit_BMP280::SAMPLING_X2,
                  Adafruit_BMP280::SAMPLING_X16,
                  Adafruit_BMP280::FILTER_X16,
                  Adafruit_BMP280::STANDBY_MS_500);
  iteration_count = 0;
}

void loop() {
  loop_start = millis();

  if (wifiMulti.run() != WL_CONNECTED) {
    Serial.println("Not connected to WiFi!");
    delay(1000);
    return;
  }

  pointDevice.clearFields();
  timesPointDevice.clearFields();

  start_reading = millis();

  double aht20temp = (double) AHT20.readTemperature();
  pointDevice.addField("aht20temperature", aht20temp);

  double aht20hum = (double) AHT20.readHumidity();
  pointDevice.addField("aht20humidity", aht20hum);

  double bmptemp = (double) bmp.readTemperature();
  pointDevice.addField("bmptemperature", bmptemp);

  double bmppress = (double) bmp.readPressure();
  pointDevice.addField("bmppressure", bmppress);

  double bmpalt = (double) bmp.readAltitude();
  pointDevice.addField("bmpalt", bmpalt);
  // TODO: Implement reading and adding a field for sensor values, for each
  //       other value that is being recorded.
  //       Functions for reading those values are:
  //         AHT20.readHumidity()
  //         bmp.readTemperature()
  //         bmp.readPressure()
  //         bmp.readAltitude()
  //
  //       We recommend using and modifying the previously implemented (above
  //       this comment section).

  start_sending = millis();

  if (!client.writePoint(pointDevice)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  iteration_complete = millis();
  iteration_count++;

  timesPointDevice.addField("rssi", WiFi.RSSI());
  timesPointDevice.addField("read", start_calculating - start_reading);
  timesPointDevice.addField("calc", start_sending - start_calculating);
  timesPointDevice.addField("send", iteration_complete - start_sending);

  if (!client.writePoint(timesPointDevice)) {
    Serial.print("InfluxDB write failed: ");
    Serial.println(client.getLastErrorMessage());
  }

  loop_end = millis();

  if (loop_end - loop_start < DELAY_BEFORE) {
    delay(DELAY_BEFORE - (loop_end - loop_start));
  }
}
