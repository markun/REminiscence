#ifndef MID_DEVICE_FLUIDSYNTH_H__
#define MID_DEVICE_FLUIDSYNTH_H__

#include "midi_device.h"

#include <fluidsynth.h>
static const uint8_t _mt32ToGm[128] = {
//        0    1    2    3    4    5    6    7    8    9    A    B    C    D    E    F
          0,   1,   0,   2,   4,   4,   5,   3,  16,  17,  18,  16,  16,  19,  20,  21, // 0x
          6,   6,   6,   7,   7,   7,   8, 112,  62,  62,  63,  63,  38,  38,  39,  39, // 1x
         88,  95,  52,  98,  97,  99,  14,  54, 102,  96,  53, 102,  81, 100,  14,  80, // 2x
         48,  48,  49,  45,  41,  40,  42,  42,  43,  46,  45,  24,  25,  28,  27, 104, // 3x
         32,  32,  34,  33,  36,  37,  35,  35,  79,  73,  72,  72,  74,  75,  64,  65, // 4x
         66,  67,  71,  71,  68,  69,  70,  22,  56,  59,  57,  57,  60,  60,  58,  61, // 5x
         61,  11,  11,  98,  14,   9,  14,  13,  12, 107, 107,  77,  78,  78,  76,  76, // 6x
         47, 117, 127, 118, 118, 116, 115, 119, 115, 112,  55, 124, 123,   0,  14, 117  // 7x
};
// Default MIDI event handlers
struct MidiDeviceFluidsynth : public MidiDevice {
    MidiDeviceFluidsynth() :
        _settings(new_fluid_settings())
    {
        fluid_settings_setstr(_settings, "audio.driver", "pulseaudio");
        fluid_settings_setstr(_settings, "audio.period-size", "4096");
        fluid_settings_setint(_settings, "audio.pulseaudio.adjust-latency", 0);
        _synth = new_fluid_synth(_settings);
        fluid_synth_sfload(_synth, "/files/Software/Games/Audio/Soundfonts/SGM-V2.01.sf2", 1);
        //_adriver = new_fluid_audio_driver(_settings, _synth);
    }

    ~MidiDeviceFluidsynth() {
        //delete_fluid_audio_driver(_adriver);
        delete_fluid_synth(_synth);
        delete_fluid_settings(_settings);
    }

    void NoteOn (int Chn, int Num, int Speed)
    { 
        fprintf(stderr, "%s\n", __func__);
        Chn = _mt32ToGm[Chn];
        fluid_synth_noteon(_synth, Chn, Num, Speed);
    }
    void NoteOff(int Chn, int Num, int Speed)
    {
        fprintf(stderr, "%s\n", __func__);
        Chn = _mt32ToGm[Chn];
        fluid_synth_noteoff(_synth, Chn, Num);
    }
    void KAfter (int Chn, int Num, int Speed) { fprintf(stderr, "%s\n", __func__); }
    void CChange(int Chn, int Num, int Speed)
    {
        fprintf(stderr, "%s\n", __func__);
        Chn = _mt32ToGm[Chn];
        fluid_synth_cc(_synth, Chn, Num, Speed);
    }
    void PChange(int Chn, int Num)
    {
        fprintf(stderr, "%s\n", __func__);
        Chn = _mt32ToGm[Chn];
        fluid_synth_program_change(_synth, Chn, Num);
    }
    void CAfter (int Chn, int Num) { fprintf(stderr, "%s\n", __func__); }
    void WChange(int Chn, int Num, int Speed) { fprintf(stderr, "%s\n", __func__); }

    fluid_settings_t* _settings;
    fluid_synth_t* _synth;
    fluid_audio_driver_t* _adriver;

    bool mix(int16_t *buf, size_t len) override {
        return fluid_synth_write_s16(_synth, len / 2, buf, 0, 2, buf, 1, 2) == FLUID_OK;
    }
};

#endif // MID_DEVICE_FLUIDSYNTH_H__
