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
    // Temp (deg C), time (sec). Uses https://www.johansontechnology.com/docs/3177/johanson-reflow-profile_0ixNOve.pdf
    {200, 0, RISE},   // Preheat / Soak
    {200, 120, HOLD}, // Ramp to Peak
    {240, 0, RISE},   // Reach reflow temp, subtracted 20 for thermocouple control delay
    {240, 20, HOLD},  // Reflow, subtracted 20 for thermocouple control delay
    {0, 0, STOP}      // Cooldown, end of program
};

MAX6675 thermocouple(1, 2, 3);
Relay power(4, true);

void setup()
{
  Serial.begin(115200);
  thermocouple.begin();
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
      float currentTemp = thermocouple.getCelsius();
      while (currentTemp < step.tempCelsius)
      {
        Serial.println("Current Temp (C): " + String(currentTemp) + ", Target: " + String(step.tempCelsius) + " deg C");
        delay(500);
        currentTemp = thermocouple.getCelsius();
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
        float currentTemp = thermocouple.getCelsius();
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
}

void loop()
{
  // a safe programmer could add safety disables here (not me, on this janky mcgyver project)
}