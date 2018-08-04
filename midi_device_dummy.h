#ifndef MID_DEVICE_DUMMY_H__
#define MID_DEVICE_DUMMY_H__

#include "midi_device.h"
// Default MIDI event handlers
struct MidiDeviceDummy : public MidiDevice {
    void NoteOff(int Chn, int Num, int Speed) {}
    void NoteOn (int Chn, int Num, int Speed) {}
    void KAfter (int Chn, int Num, int Speed) {}
    void CChange(int Chn, int Num, int Speed) {}
    void PChange(int Chn, int Num) {}
    void CAfter (int Chn, int Num) {}
    void WChange(int Chn, int Num, int Speed) {}
};

#endif // MID_DEVICE_DUMMY_H__
