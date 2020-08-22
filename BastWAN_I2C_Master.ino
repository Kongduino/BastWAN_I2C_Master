#include <Wire.h>
#include "WirePacker.h"

#define I2C_SLAVE_ADDR 0x04

char buff[128];

void hexDump(char *buf, uint16_t len) {
  String s = "|", t = "| |";
  Serial.println(F("  |.0 .1 .2 .3 .4 .5 .6 .7 .8 .9 .a .b .c .d .e .f |"));
  Serial.println(F("  +------------------------------------------------+ +----------------+"));
  for (uint16_t i = 0; i < len; i += 16) {
    for (uint8_t j = 0; j < 16; j++) {
      if (i + j >= len) {
        s = s + "   "; t = t + " ";
      } else {
        char c = buf[i + j];
        if (c < 16) s = s + "0";
        s = s + String(c, HEX) + " ";
        if (c < 32 || c > 127) t = t + ".";
        else t = t + (char)c;
      }
    }
    uint8_t index = i / 16;
    Serial.print(index, HEX); Serial.write('.');
    Serial.println(s + t + "|");
    s = "|"; t = "| |";
  }
  Serial.println(F("  +------------------------------------------------+ +----------------+"));
}

void scanI2C() {
  byte error, address;
  int nDevices;
  memset(buff, 0, 128);
  Serial.println("Scanning...");
  nDevices = 0;
  for (address = 1; address < 127; address++)   {
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
    if (error == 0) {
      buff[address] = 'o';
      nDevices++;
    }
  }
  if (nDevices == 0) Serial.println("No I2C devices found\n");
  else Serial.println("Found " + String(nDevices) + "\n");
  hexDump(buff, 128);
}

void setup() {
  Serial.begin(115200); // start serial for output
  delay(1000);
  Serial.flush();
  Serial.println("\n\n\nI2C Test/Master");
  Wire.begin();
  //  Wire.setClock(400e6);
  delay(1000);
  Serial.println("SDA: " + String(SDA));
  Serial.println("SCL: " + String(SCL));
  pinMode(7, OUTPUT);
}

byte x = 0;

void loop() {
  scanI2C();
  digitalWrite(7, HIGH);
  WirePacker packer;
  // then add data the same way as you would with Wire
  packer.write("x is ");
  packer.write(x);
  Serial.print("x is ");
  Serial.println(x);
  // after adding all data you want to send, close the packet
  packer.end();
  // now transmit the packed data
  Wire.beginTransmission(I2C_SLAVE_ADDR);
  while (packer.available()) {
    // write every packet byte
    Wire.write(packer.read());
  }
  Wire.endTransmission(); // stop transmitting
  x++;
  digitalWrite(7, LOW);
  delay(5000);
}
