#include <ArduinoJson.h>

#include <Digital_Light_TSL2561.h>
#include <Digital_Light_ISL29035.h>

#include <TH02_dev.h>

#include "Arduino.h"
#include "Wire.h"

const int capacity = JSON_OBJECT_SIZE(2);
StaticJsonDocument<capacity> doc;

void setup()
{
  Serial.begin(115200);

  TH02.begin();
  TSL2561.init();
}

void loop()
{
  StaticJsonDocument<1024> doc;

  float temper = TH02.ReadTemperature();
  doc["temperature_celcius"] = temper;

  float humidity = TH02.ReadHumidity();
  doc["humidity"] = humidity;

  serializeJson(doc, Serial);

  Serial.print("\n");

  delay(5000);
}
