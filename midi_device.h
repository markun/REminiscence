#ifndef MID_DEVICE_H__
#define MID_DEVICE_H__

#include "intern.h"

struct MidiDevice {
    virtual ~MidiDevice() {};
    // Called when 'Note On' event is received
    virtual void NoteOn(int channel, int note, int velocity) = 0;

    // Called when 'Note Off' event is received
    virtual void NoteOff(int channel, int note, int velocity) = 0;

    // Called when 'Key aftertouch' event is received
    virtual void KAfter(int channel, int note, int velocity) = 0;

    // Called when 'Control change' event is received
    virtual void CChange(int channel, int control, int value) = 0;

    // Called when 'Path change' event is received
    virtual void PChange(int channel, int program) = 0;

    // Called when 'Channel aftertouch' event is received
    virtual void CAfter(int channel, int pressure) = 0;

    // Called when 'Pitch change' event is received
    virtual void WChange(int channel, int lsb, int msb) = 0;

    virtual bool mix(int16_t *buf, size_t len) = 0;
};

#endif // MID_DEVICE_H__
