// Windows MIDI output driver
#define WIN32_LEAN_AND_MEAN 1
#define NOGDI 1
#include <windows.h>
#include <mmsystem.h>

#define MAKE_SHORT_MSG(_0, _1, _2, _3) \
    (((0) << 24) | ((_3) << 16) | ((_2) << 8) | (_0) | (_1))

// Windows MIDI event handlers
// (These should be moved outside the mid_player module).
static HMIDIOUT out;

void MidiDeviceWin::MidiDeviceWin() {
    // Initialize MIDI event handlers
    // (These lines should be moved outside the mid_player module).
    unsigned int err;
 
    err = midiOutOpen(&out, MIDI_MAPPER, 0, 0, CALLBACK_NULL);
    if (err != MMSYSERR_NOERROR)
    {
//       printf("error opening MIDI Mapper: %d\n", err);
        return -1;
    }
 
    return 0;
}

void MidiDeviceWin::~MidiDeviceWin()
{
    midiOutClose(out);
}

void MidiDeviceWin::NoteOff(int Chn, int Num, int Speed)
{
    midiOutShortMsg(out, MAKE_SHORT_MSG(0x80, Chn, Num, Speed));
}

void MidiDeviceWin::NoteOn(int Chn, int Num, int Speed)
{
    midiOutShortMsg(out, MAKE_SHORT_MSG(0x90, Chn, Num, Speed));
}

void MidiDeviceWin::KAfter(int Chn, int Num, int Speed)
{
    midiOutShortMsg(out, MAKE_SHORT_MSG(0xA0, Chn, Num, Speed));
}

void MidiDeviceWin::CChange(int Chn, int Num, int Speed)
{
    midiOutShortMsg(out, MAKE_SHORT_MSG(0xB0, Chn, Num, Speed));
}

void MidiDeviceWin::PChange(int Chn, int Num)
{
    midiOutShortMsg(out, MAKE_SHORT_MSG(0xC0, Chn, Num, 0));
}

void MidiDeviceWin::CAfter(int Chn, int Num)
{
    midiOutShortMsg(out, MAKE_SHORT_MSG(0xD0, Chn, Num, 0));
}

void MidiDeviceWin::WChange(int Chn, int Num, int Speed)
{
    midiOutShortMsg(out, MAKE_SHORT_MSG(0xE0, Chn, Num, Speed));
}
// END of Windows MIDI output driver


