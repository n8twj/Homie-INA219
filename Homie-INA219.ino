#include <Homie.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 battery(0x40);
Adafruit_INA219 solar(0x41);

float shuntVoltage = 0;
float busVoltage = 0;
float currentMa = 0;
float loadVoltage = 0;

const int sleepTimeS = 300;
unsigned long lastSent = 0;

HomieNode BatteryLoadVoltageNode("batteryLoadVoltage", "voltage");
HomieNode BatteryCurrentNode("batteryCurrent", "millamps");
HomieNode SolarLoadVoltageNode("solarLoadVoltage", "voltage");
HomieNode SolarCurrentNode("solarCurrent", "millamps");

void setupHandler() {
  Homie.setNodeProperty(BatteryLoadVoltageNode, "unit", "Volts", true);
  Homie.setNodeProperty(BatteryCurrentNode, "unit", "millamps", true);
  Homie.setNodeProperty(SolarLoadVoltageNode, "unit", "Volts", true);
  Homie.setNodeProperty(SolarCurrentNode, "unit", "millamps", true);
}

void loopHandler() {
    shuntVoltage = solar.getShuntVoltage_mV();
    busVoltage = solar.getBusVoltage_V();
    currentMa = solar.getCurrent_mA();
    loadVoltage = busVoltage + (shuntVoltage / 1000);

    Homie.setNodeProperty(SolarLoadVoltageNode, "volts", String(loadVoltage), true);
    Homie.setNodeProperty(SolarCurrentNode, "milliamps", String(currentMa), true);

    shuntVoltage = battery.getShuntVoltage_mV();
    busVoltage = battery.getBusVoltage_V();
    currentMa = battery.getCurrent_mA();
    loadVoltage = busVoltage + (shuntVoltage / 1000);

    Homie.setNodeProperty(BatteryLoadVoltageNode, "volts", String(loadVoltage), true);
    Homie.setNodeProperty(BatteryCurrentNode, "milliamps", String(currentMa), true);
    Homie.prepareForSleep();
}

void onHomieEvent(HomieEvent event) {
  switch(event) {
    case HomieEvent::READY_FOR_SLEEP:
      Serial.println("Sleepy time");
      ESP.deepSleep(sleepTimeS * 1000000);
      break;
  }
}

void setup() {
  battery.begin();
  solar.begin();
  Homie.setFirmware("ina219", "1.0.1");
  Homie.registerNode(SolarLoadVoltageNode);
  Homie.registerNode(SolarCurrentNode);
  Homie.registerNode(BatteryLoadVoltageNode);
  Homie.registerNode(BatteryCurrentNode);
  Homie.setSetupFunction(setupHandler);
  Homie.onEvent(onHomieEvent);
  Homie.setLoopFunction(loopHandler);
  Homie.setup();
}

void loop() {
  Homie.setLedPin(12, LOW);
  Homie.loop();
}
