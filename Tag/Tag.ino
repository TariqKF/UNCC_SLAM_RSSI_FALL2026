#include <SPI.h>
#include "DW1000Ranging.h"

#define SPI_SCK  18
#define SPI_MISO 19
#define SPI_MOSI 23
const uint8_t PIN_SS  = 4;
const uint8_t PIN_RST = 27;
const uint8_t PIN_IRQ = 34;

// This tag's unique address (Tag = Red)
#define TAG_ADD "7D:00:22:EA:82:60:3B:9C"

// ---------------- Telemetry packets ----------------
// Range reading (one line each time any anchor reports a range):
//   $UWB,<seq>,<ms>,<anchor>,<shortId>,<range_m>,<rx_dbm>*<checksum>
// Event (anchor found / lost):
//   $EVT,<seq>,<ms>,<FOUND|LOST>,<anchor>,<shortId>*<checksum>
//
// seq      = packet counter, +1 every packet (gaps = lost packets)
// ms       = millis() on the tag
// checksum = XOR of every character between '$' and '*', 2 hex digits
//            (same idea as GPS/NMEA, lets the laptop reject corrupted lines)

uint32_t seq = 0;

// Short ID -> anchor letter
//   A = Green (85)   B = Pink (86)   C = Blue (87)
char anchorLabel(uint16_t shortId) {
  switch (shortId) {
    case 0x1785: return 'A';
    case 0x1786: return 'B';
    case 0x1787: return 'C';
    default:     return '?';
  }
}

// Adds the '$', checksum and newline around a packet body and prints it
void sendPacket(const char* body) {
  uint8_t cs = 0;
  for (const char* p = body; *p; p++) cs ^= (uint8_t)*p;
  Serial.printf("$%s*%02X\n", body, cs);
}

void newRange() {
  DW1000Device* dev = DW1000Ranging.getDistantDevice();
  uint16_t id = dev->getShortAddress();
  char body[80];
  snprintf(body, sizeof(body), "UWB,%lu,%lu,%c,%04X,%.3f,%.2f",
           (unsigned long)seq++, (unsigned long)millis(),
           anchorLabel(id), id, dev->getRange(), dev->getRXPower());
  sendPacket(body);
}

void sendEvent(const char* what, DW1000Device* device) {
  uint16_t id = device->getShortAddress();
  char body[64];
  snprintf(body, sizeof(body), "EVT,%lu,%lu,%s,%c,%04X",
           (unsigned long)seq++, (unsigned long)millis(),
           what, anchorLabel(id), id);
  sendPacket(body);
}

void newDevice(DW1000Device* device)      { sendEvent("FOUND", device); }
void inactiveDevice(DW1000Device* device) { sendEvent("LOST",  device); }

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
