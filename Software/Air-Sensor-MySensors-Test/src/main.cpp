#include <Arduino.h>
#include "Adafruit_SHT4x.h"
#include "Adafruit_SGP40.h"

/////// MYSENORS SETTINGS ///////
#define MY_DEBUG
#define MY_RADIO_NRF5_ESB
#define CHILD_ID_TEMP 1
#define CHILD_ID_HUM 2
#define CHILD_ID_IAQ 3
#define SN "air-quality"
#define SV "1.0"
// #define MY_NODE_ID 193

/////// PIN DEFINITIONS ///////
#define CAP_VOLTAGE 16 // P0.05 => D16
#define VOLT_LOW 38    // P1.09 => D38

/////// VOLTAGE MEASUREMENT ///////
#define ADC_FACTOR (0.73242188F)  // 3V / 4096
#define RESISTOR_DIVIDER (678.0F) // this factor has to be determined for every individual device

#include <MySensors.h>

/////// INTERNALS ///////
MyMessage temp_msg(CHILD_ID_TEMP, V_TEMP);
MyMessage hum_msg(CHILD_ID_HUM, V_HUM);
MyMessage iaq_msg(CHILD_ID_IAQ, V_LEVEL);
bool sentInitialValue = false;

bool powerDown = false;

long lastsent = 0;

float lastTemp, lastHum, lastIAQ;

Adafruit_SGP40 sgp;
Adafruit_SHT4x sht4 = Adafruit_SHT4x();

uint32_t i;

void LOW_VOLTAGE_ISR()
{
  powerDown = true;
  Serial.println("----- POWER DOWN!!! -----");
}

void presentation()
{
  sendSketchInfo(SN, SV);
  present(CHILD_ID_TEMP, S_TEMP);
  present(CHILD_ID_HUM, S_HUM);
  present(CHILD_ID_IAQ, S_AIR_QUALITY);
}

void setup()
{

  attachInterrupt(VOLT_LOW, LOW_VOLTAGE_ISR, FALLING);

  // put your setup code here, to run once:
  if (!sht4.begin())
  {
    Serial.println("Couldn't find SHT4x");
    while (1)
      delay(1);
  }
  Serial.println("Found SHT4x sensor");
  Serial.print("Serial number 0x");
  Serial.println(sht4.readSerial(), HEX);
  sht4.setPrecision(SHT4X_MED_PRECISION);
  sht4.setHeater(SHT4X_NO_HEATER);

  if (!sgp.begin())
  {
    Serial.println("SGP40 sensor not found :(");
    while (1)
      ;
  }
  Serial.print("Found SHT3x + SGP40 serial #");
  Serial.print(sgp.serialnumber[0], HEX);
  Serial.print(sgp.serialnumber[1], HEX);
  Serial.println(sgp.serialnumber[2], HEX);
}

void loop()
{
  // For Home Assistant, the first message has to be sent from the loop.
  sensors_event_t humidity, temp;
  sht4.getEvent(&humidity, &temp); // populate temp and humidity objects with fresh data

  if (!sentInitialValue)
  {
    send(temp_msg.set(22, 3)); // send dummy value
    send(hum_msg.set(50, 3));  // send dummy value
    send(iaq_msg.set(100, 0)); // send dummy value
    sendBatteryLevel(50);
    wait(2000);
    sentInitialValue = true;

    for (int n = 0; n <= 230; n++)
    {
      float t = temp.temperature;
      float h = humidity.relative_humidity;
      uint32_t voc_index;
      voc_index = sgp.measureVocIndex(t, h);
      Serial.println(voc_index);
      delay(50);
    }
    Serial.println("finished setup");
  }

  if (!powerDown)
  {
    sensors_event_t humidity, temp;
    sht4.getEvent(&humidity, &temp); // populate temp and humidity objects with fresh data

    float t = temp.temperature;
    float h = humidity.relative_humidity;
    Serial.print("T: ");
    Serial.println(t);
    Serial.print("H: ");
    Serial.println(h);

    uint32_t voc_index;

    voc_index = sgp.measureVocIndex(t, h);
    Serial.print("Voc Index: ");
    Serial.println(voc_index);

    long now = millis();

    if (now >= lastsent + 30000)
    {
      lastsent = now;
      if (t > (lastTemp + 0.5) || t < (lastTemp - 0.5))
      {
        send(temp_msg.set(t, 3));
        lastTemp = t;
      }
      if (h > (lastHum + 3) || h < (lastHum - 3))
      {
        send(hum_msg.set(h, 3));
        lastHum = h;
      }
      if (voc_index > lastIAQ + 2 || voc_index < lastIAQ - 2)
      {
        send(iaq_msg.set(voc_index, 0));
        lastIAQ = voc_index;
      }
    }
  }
  sleep(900000); // 15 Minutes
}