#include <ArduinoJson.h>

#include <Digital_Light_TSL2561.h>
#include <Digital_Light_ISL29035.h>

#include <TH02_dev.h>

#include "Arduino.h"
#include "Wire.h"

const int capacity = JSON_OBJECT_SIZE(2);
StaticJsonDocument<capacity> doc;

// JsonObject temperature_celcius = doc.createNestedObject("temperature_celcius");
// JsonObject humidity = doc.createNestedObject("humidity");

void setup()
{
  Serial.begin(9600); // start serial for output
  Serial.println("Slim bob! :)\n");
  
  TH02.begin();
  TSL2561.init();
}

void loop()
{
  // StaticJsonDocument<200> doc;
  // JsonArray array = doc.to<JsonArray>();
  // JsonObject nested = array.createNestedObject();
  // nested["hello"] = "world";
  // serializeJson(array, Serial);

  float temper = TH02.ReadTemperature();  
  doc["temperature_celcius"] = temper;
 
  float humidity = TH02.ReadHumidity(); 
  doc["humidity"] = humidity;  
  
  serializeJson(doc, Serial);

  delay(5000);
}
