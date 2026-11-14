#include <SPI.h>
#include "DW1000Ranging.h"

#define SPI_SCK  18
#define SPI_MISO 19
#define SPI_MOSI 23
const uint8_t PIN_SS  = 4;
const uint8_t PIN_RST = 27;
const uint8_t PIN_IRQ = 34;

// This tag's unique address
#define TAG_ADD "7D:00:22:EA:82:60:3B:9C"

void newRange() {
  Serial.print("from: ");
  Serial.print(DW1000Ranging.getDistantDevice()->getShortAddress(), HEX);
  Serial.print("\t Range: ");
  Serial.print(DW1000Ranging.getDistantDevice()->getRange());
  Serial.print(" m\t RX power: ");
  Serial.print(DW1000Ranging.getDistantDevice()->getRXPower());
  Serial.println(" dBm");
}

void newDevice(DW1000Device* device) {
  Serial.print("Anchor found: ");
  Serial.println(device->getShortAddress(), HEX);
}

void inactiveDevice(DW1000Device* device) {
  Serial.print("Anchor lost: ");
  Serial.println(device->getShortAddress(), HEX);
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  SPI.begin(SPI_SCK, SPI_MISO, SPI_MOSI);
  DW1000Ranging.initCommunication(PIN_RST, PIN_SS, PIN_IRQ);
  DW1000Ranging.attachNewRange(newRange);
  DW1000Ranging.attachNewDevice(newDevice);
  DW1000Ranging.attachInactiveDevice(inactiveDevice);
  DW1000Ranging.startAsTag(TAG_ADD, DW1000.MODE_LONGDATA_RANGE_LOWPOWER);
  Serial.println("Tag ready");
}

void loop() {
  DW1000Ranging.loop();
}