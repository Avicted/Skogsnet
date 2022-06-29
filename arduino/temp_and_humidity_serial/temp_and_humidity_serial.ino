#include <Digital_Light_TSL2561.h>
#include <Digital_Light_ISL29035.h>

#include <TH02_dev.h>

#include "Arduino.h"
#include "Wire.h"

#define ShowSerial Serial


void setup() {
  ShowSerial.begin(9600);        // start serial for output

  ShowSerial.println("****TH06_dev demo by seeed studio****\n");
  
  TH02.begin();

  TSL2561.init();

  /* Determine TH02_dev is available or not */
  ShowSerial.println("TH02_dev is available.\n");
}


void loop() {
  float temper = TH02.ReadTemperature();
  ShowSerial.print("Temperature: ");
  ShowSerial.print(temper);
  ShowSerial.println("C\r\n");
  //Serial.print("%\t");

  float humidity = TH02.ReadHumidity();
  ShowSerial.print("Humidity: ");
  ShowSerial.print(humidity);
  ShowSerial.println("%\r\n");

  // ShowSerial.print("The Light value is: ");
  // ShowSerial.println(TSL2561.readVisibleLux());
  delay(1000);
}
