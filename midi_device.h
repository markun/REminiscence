#ifndef MID_DEVICE_H__
#define MID_DEVICE_H__

#include "intern.h"

struct MidiDevice {
    virtual ~MidiDevice() {};
    // Called when 'Note On' event is received
    virtual void NoteOn(int Chn, int Num, int Speed) = 0;

    // Called whe'Note Off' event is received
    virtual void NoteOff(int Chn, int Num, int Speed) = 0;

    // Called whe'Key aftertouch' event is received
    virtual void KAfter(int Chn, int Num, int Speed) = 0;

    // Called whe'Control change' event is received
    virtual void CChange(int Chn, int Num, int Speed) = 0;

    // Called whe'Path change' event is received
    virtual void PChange(int Chn, int Num) = 0;

    // Called whe'Channel aftertouch' event is received
    virtual void CAfter(int Chn, int Num) = 0;

    // Called whe'Pitch change' event is received
    virtual void WChange(int Chn, int Num, int Speed) = 0;
};

#endif // MID_DEVICE_H__
