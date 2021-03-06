#include <Homie.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

Adafruit_INA219 battery(0x40);
Adafruit_INA219 solar(0x41);

float shuntVoltage = 0;
float busVoltage = 0;
float currentMa = 0;
float loadVoltage = 0;

const int sleepTimeS = 60;
bool sentOnce = false;

HomieNode BatteryLoadVoltageNode("batteryLoadVoltage", "voltage");
HomieNode BatteryCurrentNode("batteryCurrent", "millamps");
HomieNode SolarLoadVoltageNode("solarLoadVoltage", "voltage");
HomieNode SolarCurrentNode("solarCurrent", "millamps");

void setupHandler() {
  Homie.setNodeProperty(BatteryLoadVoltageNode, "unit").setRetained(true).send("volts");
  Homie.setNodeProperty(BatteryCurrentNode, "unit").setRetained(true).send("millamps");
  Homie.setNodeProperty(SolarLoadVoltageNode, "unit").setRetained(true).send("volts");
  Homie.setNodeProperty(SolarCurrentNode, "unit").setRetained(true).send("millamps");
}

void loopHandler() {
  if (!sentOnce) {
    sentOnce = true;
    shuntVoltage = solar.getShuntVoltage_mV();
    busVoltage = solar.getBusVoltage_V();
    currentMa = solar.getCurrent_mA();
    loadVoltage = busVoltage + (shuntVoltage / 1000);
    if (Homie.isConnected()) {
      Homie.setNodeProperty(SolarLoadVoltageNode, "volts").send(String(loadVoltage));
      Homie.setNodeProperty(SolarCurrentNode, "milliamps").send(String(currentMa));
    }
    shuntVoltage = battery.getShuntVoltage_mV();
    busVoltage = battery.getBusVoltage_V();
    currentMa = battery.getCurrent_mA();
    loadVoltage = busVoltage + (shuntVoltage / 1000);
    if (Homie.isConnected()) {
      Homie.setNodeProperty(BatteryLoadVoltageNode, "volts").send(String(loadVoltage));
      Homie.setNodeProperty(BatteryCurrentNode, "milliamps").send(String(currentMa));
    }
  }
}

void onHomieEvent(HomieEvent event) {
  switch(event) {
    case HomieEvent::MQTT_CONNECTED:
      sentOnce = false;
      Homie.prepareForSleep();
      break;
    case HomieEvent::READY_FOR_SLEEP:
      ESP.deepSleep(sleepTimeS * 1000000, RF_NO_CAL);
      break;
  }
}

void setup() {
  Serial.begin(115200);
  battery.begin();
  solar.begin();
  Homie_setFirmware("ina219", "1.0.2");
  Homie.setSetupFunction(setupHandler);
  Homie.setLoopFunction(loopHandler);
  Homie.onEvent(onHomieEvent);
  Homie.disableLogging();
  Homie.disableLedFeedback();

  BatteryLoadVoltageNode.advertise("unit");
  BatteryLoadVoltageNode.advertise("voltage");
  BatteryCurrentNode.advertise("unit");
  BatteryCurrentNode.advertise("milliamps");

  SolarLoadVoltageNode.advertise("unit");
  SolarLoadVoltageNode.advertise("voltage");
  SolarCurrentNode.advertise("unit");
  SolarCurrentNode.advertise("milliamps");

  Homie.setup();
}

void loop() {
  Homie.loop();
}
