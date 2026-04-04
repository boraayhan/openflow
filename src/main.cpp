// Author: Bora Ayhan
// Not my finest code work, I cooked this up alongside my morning coffee. I just needed this to work for me lol
// Licensed under a BORA License -> Do not use this unless you're Bora and are willing to deal with the headache

#include <Arduino.h>
#include "Relay.h"
#include "MAX6675.h"

enum StepMode
{
  RISE,
  HOLD,
  STOP
};

struct ReflowStep
{
  double tempCelsius;
  unsigned long timeSeconds;
  StepMode mode;
};

ReflowStep steps[] = {
    // Temp (deg C), time (sec). Uses a custom profile
    {120, 0, RISE},  // Preheat / Soak
    {120, 60, RISE}, // Warm up board
    {140, 0, RISE},  // Reach reflow temp, subtracted 20 for thermocouple control delay
    {140, 20, HOLD}, // Reflow, subtracted 20 for thermocouple control delay
    {0, 0, STOP}     // Cooldown, end of program
};

int thermoDO = 4;
int thermoCS = 5;
int thermoCLK = 6;

MAX6675 thermocouple(thermoCLK, thermoCS, thermoDO);

Relay power(7, false);

void setup()
{
  Serial.begin(115200);
  power.begin();
  Serial.println("Press any key to start the reflow oven...");
  while (!Serial.available())
  {
  }
  Serial.println("Starting reflow in 5 seconds");
  delay(5000);

  for (const ReflowStep &step : steps)
  {
    if (step.mode == STOP)
    {
      power.turnOff();
      break;
    }
    if (step.mode == RISE)
    {
      float currentTemp = thermocouple.readCelsius();
      while (currentTemp < step.tempCelsius)
      {
        Serial.println("Current Temp (C): " + String(currentTemp) + ", Target: " + String(step.tempCelsius) + " deg C");
        delay(500);
        currentTemp = thermocouple.readCelsius();
        if (currentTemp > step.tempCelsius + 2)
        {
          power.turnOff();
        }
        if (currentTemp < step.tempCelsius - 2)
        {
          power.turnOn();
        }
      }
    }
    else if (step.mode == HOLD)
    {
      unsigned long startTime = millis();
      while (millis() - startTime < step.timeSeconds * 1000UL)
      {
        float currentTemp = thermocouple.readCelsius();
        Serial.println("Current Temp (C): " + String(currentTemp) + ", Target: " + String(step.tempCelsius) + ", Time Remaining: " + String((step.timeSeconds * 1000UL - (millis() - startTime)) / 1000UL) + " sec.");

        if (currentTemp > step.tempCelsius + 2)
        {
          power.turnOff();
        }
        if (currentTemp < step.tempCelsius - 2)
        {
          power.turnOn();
        }
        delay(500);
      }
    }
  }
  Serial.println("Reflow complete. Cooldown initiated.");
  while(true) {
    Serial.println("Current Temp (C): " + String(thermocouple.readCelsius()) + ", Cooling down...");
    delay(3000);
  }
}

void loop()
{
  // a safe programmer could add safety disables here (not me, on this janky mcgyver project)
}