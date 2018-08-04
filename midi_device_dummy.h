#ifndef MID_DEVICE_DUMMY_H__
#define MID_DEVICE_DUMMY_H__

#include "midi_device.h"
// Default MIDI event handlers
struct MidiDeviceDummy : public MidiDevice {
    void NoteOn (int Chn, int Num, int Speed) { fprintf(stderr, "%s\n", __func__); }
    void NoteOff(int Chn, int Num, int Speed) { fprintf(stderr, "%s\n", __func__); }
    void KAfter (int Chn, int Num, int Speed) { fprintf(stderr, "%s\n", __func__); }
    void CChange(int Chn, int Num, int Speed) { fprintf(stderr, "%s\n", __func__); }
    void PChange(int Chn, int Num) { fprintf(stderr, "%s\n", __func__); }
    void CAfter (int Chn, int Num) { fprintf(stderr, "%s\n", __func__); }
    void WChange(int Chn, int Num, int Speed) { fprintf(stderr, "%s\n", __func__); }
};

#endif // MID_DEVICE_DUMMY_H__
