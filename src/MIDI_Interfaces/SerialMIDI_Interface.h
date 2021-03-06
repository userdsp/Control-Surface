#ifndef SERIALMIDI_INTERFACE_H_
#define SERIALMIDI_INTERFACE_H_

#include "MIDI_Interface.h"
#include "../MIDI_Parsers/SerialMIDI_Parser.h"

class StreamMIDI_Interface : public MIDI_Interface
{
public:
  StreamMIDI_Interface(Stream &stream) : MIDI_Interface(parser), stream(stream) {}

  virtual MIDI_read_t read()
  {
    while (stream.available() > 0)
    {
      uint8_t midiByte = stream.read();
#ifdef DEBUG
      Serial.print("New byte:\t");
      Serial.println(midiByte, HEX);
#endif
      MIDI_read_t parseResult = parser.parse(midiByte);
      if (parseResult != NO_MESSAGE)
        return parseResult;
    }
    return NO_MESSAGE;
  }

protected:
  SerialMIDI_Parser parser;

  virtual void sendImpl(uint8_t m, uint8_t c, uint8_t d1, uint8_t d2)
  {
    stream.write(m | c); // Send the MIDI message over the stream
    stream.write(d1);
    stream.write(d2);
    stream.flush();
  }
  virtual void sendImpl(uint8_t m, uint8_t c, uint8_t d1)
  {
    stream.write(m | c); // Send the MIDI message over the stream
    stream.write(d1);
    stream.flush();
  }

protected:
  Stream &stream;

};

template <typename T>
class SerialMIDI_Interface : public StreamMIDI_Interface
{
public:
  SerialMIDI_Interface(T &serial, unsigned long baud) : serial(serial), baud(baud), StreamMIDI_Interface(serial) {}
  virtual void begin()
  {
    serial.begin(baud);
  }

private:
  T &serial;
  const unsigned long baud;
};

class HardwareSerialMIDI_Interface : public SerialMIDI_Interface<HardwareSerial>
{
public:
  HardwareSerialMIDI_Interface(HardwareSerial &serial, unsigned long baud) : SerialMIDI_Interface(serial, baud) {}
};

// The Serial USB connection
// (Why do I have to do this, why don't they all inherit from one single class?)

#if !(defined(USBCON) || defined(CORE_TEENSY)) // Boards without a USB connection (UNO, MEGA, Nano ...)
class USBSerialMIDI_Interface : public HardwareSerialMIDI_Interface
{
public:
  USBSerialMIDI_Interface(unsigned long baud) : HardwareSerialMIDI_Interface(Serial, baud) {}
};
#elif defined(CORE_TEENSY)      // Teensies
class USBSerialMIDI_Interface : public SerialMIDI_Interface<usb_serial_class>
{
public:
  USBSerialMIDI_Interface(unsigned long baud) : SerialMIDI_Interface(Serial, baud) {}
};
#elif defined(ARDUINO_ARCH_SAM) // Arduino DUE
class USBSerialMIDI_Interface : public SerialMIDI_Interface<UARTClass>
{
public:
  USBSerialMIDI_Interface(unsigned long baud) : SerialMIDI_Interface(Serial, baud) {}
};
#else                           // Others (Leonardo, Micro ... )
class USBSerialMIDI_Interface : public SerialMIDI_Interface<Serial_>
{
public:
  USBSerialMIDI_Interface(unsigned long baud) : SerialMIDI_Interface(Serial, baud) {}
};
#endif

// Class for Hairless MIDI
class HairlessMIDI_Interface : public USBSerialMIDI_Interface
{
public:
  HairlessMIDI_Interface() : USBSerialMIDI_Interface(HAIRLESS_BAUD){};
};

#if defined(__AVR__) || defined(CORE_TEENSY)
#include <SoftwareSerial.h>
class SoftwarSerialMIDI_Interface : public SerialMIDI_Interface<SoftwareSerial>
{
public:
  SoftwarSerialMIDI_Interface(SoftwareSerial &serial, unsigned long baud) : SerialMIDI_Interface(serial, baud) {}
};
#endif

#endif // SERIALMIDI_INTERFACE_H_