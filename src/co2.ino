PRODUCT_ID(14399);
PRODUCT_VERSION(1);

/*
  Sketch CO2, humidity and temperature from the SCD30
  By: Sergi Trilles
  Universitat Jaume I
  Date: May 219th, 2021
*/

#include <Grove_OLED_128x64.h>
#include <JsonParserGeneratorRK.h>

#include <Wire.h>
#include "SparkFun_SCD30_Arduino_Library.h"
#include <avr/pgmspace.h>

#define FIVE_MINUTES_MILLIS (5 * 60 * 1000)
#define ONE_DAY_MILLIS (24 * 60 * 60 * 1000)

SYSTEM_THREAD(ENABLED);

bool keepAlive_set = false;
FuelGauge fuel;
char dev_name[32] = "";
String ParticleId;
bool mqConnected, publishName = false;
int previousTime = 0;
unsigned long lastSync = millis();
bool hasSetKeepAlive = false;
SCD30 airSensor;

void createEventPayload(float temp, float humidity, int light, double battery, char dname[32]) {
  JsonWriterStatic < 256 > jw; {
    JsonWriterAutoObject obj(&jw);
    jw.insertKeyValue("temp", temp);
    jw.insertKeyValue("hum", humidity);
    jw.insertKeyValue("ligh", light);
    jw.insertKeyValue("bat", battery);
    jw.insertKeyValue("dev", dname);
  }
  Particle.publish("googleDocs2", jw.getBuffer(), 60, PRIVATE);
}

void setup() {

  Wire.begin();
  Time.zone(2);
  Serial.begin(9600);
  Serial.println("SCD30 Example");

  SeeedOled.init();

  Particle.connect();
  waitUntil(Particle.connected);

  ParticleId = System.deviceID();
  Particle.subscribe("particle/device/name", handler);
  Particle.publish("particle/device/name");
  for (uint32_t ms = millis(); millis() - ms < 3000 && !publishName; Particle.process());
  Particle.publish("Particle name", dev_name, PRIVATE);
  airSensor.begin(); 

}

void loop() {

  if (!keepAlive_set && Particle.connected()) {
    Particle.keepAlive(30);
    keepAlive_set = true;
  }
  
  if ((Time.minute() % 5) == 0)
    {
        
        RGB.control(true);
        RGB.color(255, 255, 255);
        delay(4000);
        RGB.brightness(64);
        delay(1000);
        RGB.control(false);

        if (airSensor.dataAvailable()) {
        
            createEventPayload(airSensor.getTemperature(), airSensor.getHumidity(), airSensor.getCO2(), fuel.getNormalizedSoC(), dev_name);
          } else {
            Serial.println("No data");
            Particle.publish("data", "Problema obtindre datos", PRIVATE);
          }
		  
          delay(60000);
    }
    
      if (millis() - lastSync > ONE_DAY_MILLIS) {
    Particle.syncTime();
    lastSync = millis();
  }

}

void handler(const char * topic,
  const char * data) {
  strncpy(dev_name, data, sizeof(dev_name) - 1);
}