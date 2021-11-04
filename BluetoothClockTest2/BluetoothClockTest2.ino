#include "RTClib.h"
RTC_PCF8523 rtc;
#include <string.h>
#include <Arduino.h>
#include <SPI.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_SPI.h"
#include "Adafruit_BluefruitLE_UART.h"
#include "BluefruitConfig.h"
#define FACTORYRESET_ENABLE         1
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"
Adafruit_BluefruitLE_SPI ble(BLUEFRUIT_SPI_CS, BLUEFRUIT_SPI_IRQ, BLUEFRUIT_SPI_RST);
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}
uint8_t readPacket(Adafruit_BLE *ble, uint16_t timeout);
float parsefloat(uint8_t *buffer);
void printHex(const uint8_t * data, const uint32_t numBytes);
extern uint8_t packetbuffer[];
int hrs = 1;
int mins = 0;
int secs = 0;
void setup () {
  Serial.begin(115200);
  if (! rtc.begin()) {
    Serial.flush();
    abort();
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  rtc.start();
  /*float drift = 43;
    float period_sec = (7 * 86400);
    float deviation_ppm = (drift / period_sec * 1000000);
    float drift_unit = 4.34;
    int offset = round(deviation_ppm / drift_unit);
    Serial.print("Offset is "); Serial.println(offset);
    while (!Serial);
    delay(500);*/
  if ( !ble.begin(VERBOSE_MODE) ) {
    error(F("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?"));
  }
  /*if ( FACTORYRESET_ENABLE ) {
    if ( ! ble.factoryReset() ) {
      error(F("Couldn't factory reset"));
    }
    }*/
  ble.echo(false);
  ble.info();
  ble.verbose(false);
  while (! ble.isConnected()) {
    delay(500);
  }
  if ( ble.isVersionAtLeast(MINIMUM_FIRMWARE_VERSION) ) {
    ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR);
  }
  ble.setMode(BLUEFRUIT_MODE_DATA);
  Serial.print("sdbxbvkn.zdsf");
}

void loop () {
  DateTime now = rtc.now();
  Serial.print("now.hour:");
  Serial.println(now.hour());
  Serial.print("now.minute:");
  Serial.println(now.minute());
  Serial.print("now.second:");
  Serial.println(now.second());
  Serial.println();
  uint8_t len = readPacket(&ble, BLE_READPACKET_TIMEOUT);
  if (len == 0) return;
  //printHex(packetbuffer, len);
  if (packetbuffer[1] == 'B') {
    uint8_t buttonnum = packetbuffer[2] - '0';
    boolean pressed = packetbuffer[3] - '0';
    if (packetbuffer[3] == 0x31 && packetbuffer[2] == 0x37) {
      secs++;
      if (secs > 59) secs = 0;
      rtc.adjust(DateTime(2021, 1, 1, now.hour(), now.minute(), secs));
      rtc.start();
    }
    if (packetbuffer[3] == 0x31 && packetbuffer[2] == 0x38) {
      mins++;
      if (mins > 59) mins = 0;
      rtc.adjust(DateTime(2021, 1, 1, now.hour(), mins, now.second()));
      rtc.start();
    }
    if (packetbuffer[3] == 0x31 && packetbuffer[2] == 0x35) {
      hrs++;
      if (hrs > 24) hrs = 1;
      rtc.adjust(DateTime(2021, 1, 1, hrs, now.minute(), now.second()));
      rtc.start();
    }
  }
}
