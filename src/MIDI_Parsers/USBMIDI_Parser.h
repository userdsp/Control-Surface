#include "MIDI_Parser.h"

class USBMIDI_Parser : public MIDI_Parser {
  public:
    MIDI_read_t parse(uint8_t *packet);
};
