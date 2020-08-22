/**
   @file WirePacker.h
   @author Gutierrez PS <https://github.com/gutierrezps>
   @brief Class to pack the data before sending to other I2C device
   @date 2020-06-16

   WirePacker is used to pack the data before sending it to
   another I2C device, be it master->slave or slave->master.

   After creating the packer object, add data with write()
   or with Print methods such as printf(). When finished,
   call end() to close the packet.

   After that, use available() and read() methods to
   read each packet byte and send to the other device.

   Packet format:
        [0]: start byte (0x02)
        [1]: packet length
        [2]: data[0]
        [3]: data[1]
        ...
        [n+1]: data[n-1]
        [n+2]: CRC8 of packet length and data
        [n+3]: end byte (0x04)

*/
#ifndef WirePacker_h
#define WirePacker_h
#include <Arduino.h>
#include <Print.h>
#include "WireCrc.h"
#define PACKER_BUFFER_LENGTH 128

class WirePacker : public Print {
  public:
    WirePacker();
    /**
       Add a byte to the packet, only if end() was not called yet.
     * * @param data      byte to be added
       @return size_t   1 if the byte was added
    */
    size_t write(uint8_t data);
    /**
       Add a number of bytes to the packet. The number of bytes added
       may be different from quantity if the buffer becomes full.
     * * @param data      byte array to be added
       @param quantity  number of bytes to add
       @return size_t   number of bytes added
    */
    size_t write(const uint8_t *data, size_t quantity);
    inline size_t write(const char * s) {
      return write((uint8_t*) s, strlen(s));
    }
    inline size_t write(unsigned long n) {
      return write((uint8_t)n);
    }
    inline size_t write(long n) {
      return write((uint8_t)n);
    }
    inline size_t write(unsigned int n) {
      return write((uint8_t)n);
    }
    inline size_t write(int n) {
      return write((uint8_t)n);
    }
    /**
       Returns packet length so far
     * * @return size_t
    */
    size_t packetLength() const {
      if (isPacketOpen_) {
        return totalLength_ + 2;
      }
      return totalLength_;
    }
    /**
       Closes the packet. After that, use avaiable() and read()
       to get the packet bytes.
     * */
    void end();
    /**
       Returns how many packet bytes are available to be read.
     * * @return size_t
    */
    size_t available();
    /**
       Read the next available packet byte. At each call,
      the value returned by available() will be decremented.
     * * @return int  -1 if there are no bytes to be read
    */
    int read();
    /**
       Resets the packing process.
     * */
    void reset();
    /**
       Debug. Prints packet data to Serial.
     * */
    void printToSerial();

  private:
    const uint8_t frameStart_ = 0x02;
    const uint8_t frameEnd_ = 0x04;
    uint8_t buffer_[PACKER_BUFFER_LENGTH];
    uint8_t index_;
    uint8_t totalLength_;
    bool isPacketOpen_;
};
#endif
/**
   @file WirePacker.cpp
   @author Gutierrez PS <https://github.com/gutierrezps>
   @brief Class to pack the data before sending to other I2C device
   @date 2020-06-16

*/
WirePacker::WirePacker() {
  reset();
}

size_t WirePacker::write(uint8_t data) {
  if (!isPacketOpen_) {
    return 0;
  }
  // leave room for crc and end bytes
  if (totalLength_ >= PACKER_BUFFER_LENGTH - 2) {
    return 0;
  }
  buffer_[index_] = data;
  ++index_;
  totalLength_ = index_;
  return 1;
}

size_t WirePacker::write(const uint8_t *data, size_t quantity) {
  for (size_t i = 0; i < quantity; ++i) {
    if (!write(data[i])) {
      return i;
    }
  }
  return quantity;
}

void WirePacker::end() {
  isPacketOpen_ = false;
  // make room for CRC byte
  ++index_;
  buffer_[index_] = frameEnd_;
  ++index_;
  totalLength_ = index_;
  buffer_[1] = totalLength_;
  // ignore start, length, crc and end bytes
  uint8_t payloadLength = totalLength_ - 4;
  WireCrc crc8;
  crc8.calc(&totalLength_, 1); // include length in CRC
  uint8_t crc = crc8.update(buffer_ + 2, payloadLength);
  buffer_[index_ - 2] = crc;
  // prepare for reading
  index_ = 0;
}

size_t WirePacker::available() {
  if (isPacketOpen_) {
    return 0;
  }
  return totalLength_ - index_;
}

int WirePacker::read() {
  int value = -1;
  if (!isPacketOpen_ && index_ < totalLength_) {
    value = buffer_[index_];
    ++index_;
  }
  return value;
}

void WirePacker::reset() {
  buffer_[0] = frameStart_;
  index_ = 2;
  totalLength_ = 2;
  isPacketOpen_ = true;
}

void WirePacker::printToSerial() {
  printf("length: %d, ", totalLength_);
  if (isPacketOpen_) {
    Serial.print("open, ");
  } else {
    Serial.print("closed, ");
  }
  if (totalLength_ > 2) {
    Serial.print("buffer str: '");
    for (size_t i = 0; i < totalLength_; ++i) {
      if (buffer_[i] < 32 || buffer_[i] >= 127) {
        Serial.print(".");
      } else {
        Serial.print((char) buffer_[i]);
      }
    }
    Serial.print("', buffer hex: ");
    for (size_t i = 0; i < totalLength_; ++i) {
      printf("%02X ", buffer_[i]);
    }
  } else {
    Serial.print("empty");
  }
  Serial.println();
}
