#include "file.h"
#include "mixer.h"
#include "systemstub.h"
#include "util.h"

#define MAKE_SIGNATURE(_3, _2, _1, _0) \
    (((_3) << 24) | ((_2) << 16) | ((_1) << 8) | (_0))

#define SIGN_MThd   MAKE_SIGNATURE('M','T','h','d')
#define SIGN_MTrk   MAKE_SIGNATURE('M','T','r','k')

#define FRAME2MS(_frame) \
    ((_frame) * 1000UL / Player->QNoteTime)

#define META_END_OF_TRACK   0x2F
#define META_SET_TEMPO      0x51

#define MAX_TRACKS  64

typedef struct {
    unsigned int Size;
    uint8_t     *Data;
    uint32_t     DeltaTime;
    uint32_t     LastTime;
    unsigned int Index;
    uint8_t      LastCmd;

} MidiTrack;

// The MIDI File's header
typedef struct {
    uint32_t MThd;          // = "MTHd"
    uint32_t Size;
    uint16_t Format;
    uint16_t Tracks;
    uint16_t Ticks;

} MidiFileHeaderType;

// The header of a track
typedef struct {
    uint32_t MTrk;          // = "MTrk"
    uint32_t Size;

} MidiTrackHeaderType;

// Default MIDI event handlers
static void dummy_NoteOff(int Chn, int Num, int Speed) {}
static void dummy_NoteOn (int Chn, int Num, int Speed) {}
static void dummy_KAfter (int Chn, int Num, int Speed) {}
static void dummy_CChange(int Chn, int Num, int Speed) {}
static void dummy_PChange(int Chn, int Num) {}
static void dummy_CAfter (int Chn, int Num) {}
static void dummy_WChange(int Chn, int Num, int Speed) {}

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

int WinMidi_Open(void)
{
    unsigned int err;
 
    err = midiOutOpen(&out, MIDI_MAPPER, 0, 0, CALLBACK_NULL);
    if (err != MMSYSERR_NOERROR)
    {
//       printf("error opening MIDI Mapper: %d\n", err);
        return -1;
    }
 
    return 0;
}

void WinMidi_Close(void)
{
    midiOutClose(out);
}

void WinMidi_NoteOff(int Chn, int Num, int Speed)
{
    midiOutShortMsg(out, MAKE_SHORT_MSG(0x80, Chn, Num, Speed));
}

void WinMidi_NoteOn(int Chn, int Num, int Speed)
{
    midiOutShortMsg(out, MAKE_SHORT_MSG(0x90, Chn, Num, Speed));
}

void WinMidi_KAfter(int Chn, int Num, int Speed)
{
    midiOutShortMsg(out, MAKE_SHORT_MSG(0xA0, Chn, Num, Speed));
}

void WinMidi_CChange(int Chn, int Num, int Speed)
{
    midiOutShortMsg(out, MAKE_SHORT_MSG(0xB0, Chn, Num, Speed));
}

void WinMidi_PChange(int Chn, int Num)
{
    midiOutShortMsg(out, MAKE_SHORT_MSG(0xC0, Chn, Num, 0));
}

void WinMidi_CAfter(int Chn, int Num)
{
    midiOutShortMsg(out, MAKE_SHORT_MSG(0xD0, Chn, Num, 0));
}

void WinMidi_WChange(int Chn, int Num, int Speed)
{
    midiOutShortMsg(out, MAKE_SHORT_MSG(0xE0, Chn, Num, Speed));
}
// END of Windows MIDI output driver

struct MidPlayer_impl {

    unsigned int QNoteTime;
    unsigned int BaseTime;

    unsigned int TimeTicks;
    unsigned int LastTicks;
    int Tracks;

    MidiTrack Track[MAX_TRACKS];

    MidiPlay_NoteOn  NoteOn;
    MidiPlay_NoteOff NoteOff;
    MidiPlay_KAfter  KAfter;
    MidiPlay_CChange CChange;
    MidiPlay_PChange PChange;
    MidiPlay_CAfter  CAfter;
    MidiPlay_WChange WChange;

	bool _repeatIntro;

    union {
        MidiFileHeaderType  File;
        MidiTrackHeaderType Track;
    } Header;

	SystemStub *_stub;

	MidPlayer_impl() {

        // Initialize pointer to tracks
        for (int n = 0; n < MAX_TRACKS; n++)
        {
            Track[n].Data = NULL;
        }

        // Initialize default event handlers
        NoteOn  = dummy_NoteOn;
        NoteOff = dummy_NoteOff;
        KAfter  = dummy_KAfter;
        CChange = dummy_CChange;
        PChange = dummy_PChange;
        CAfter  = dummy_CAfter;
        WChange = dummy_WChange;

        // Initialize MIDI event handlers
        // (These lines should be moved outside the mid_player module).
        WinMidi_Open();

        NoteOn  = WinMidi_NoteOn;
        NoteOff = WinMidi_NoteOff;
        KAfter  = WinMidi_KAfter;
        CChange = WinMidi_CChange;
        PChange = WinMidi_PChange;
        CAfter  = WinMidi_CAfter;
        WChange = WinMidi_WChange;
	}

    int get_byte(MidiTrack *Tr)
    {
        if (Tr->Index < Tr->Size)
            return Tr->Data[Tr->Index++];
        else
            return 0;
    }

    uint32_t get_RLE(MidiTrack *Tr)
    {
        uint32_t data = 0;
        int      ch;

        do {
            ch = get_byte(Tr);

            data = (data << 7) + (ch & 0x7F);
        } while ((ch & 0x80));

        return data;
    }

    void parse(int idx)
    {
        MidiTrack *Tr = Track + idx;
        int        Chn, Cmd, ch;
        int        Event, Len, Tempo;
        int        n1, n2;

        ch = get_byte(Tr);

        if (!(ch & 0x80))
        {
            ch = Tr->LastCmd;

            if (Tr->Index > 0)
                Tr->Index--;
        } else
            Tr->LastCmd = ch;

        /* Meta-event? */
        if (ch == 0xFF)
        {
            Event = get_byte(Tr);
            Len   = get_RLE(Tr);

            switch (Event) {
            case META_END_OF_TRACK:
                Tr->Index = Tr->Size;
                break;

            case META_SET_TEMPO:
                Tempo = get_byte(Tr);
                Tempo = (Tempo << 8) | get_byte(Tr);
                Tempo = (Tempo << 8) | get_byte(Tr);

                BaseTime = Tempo / QNoteTime;
                break;

            default:
                Tr->Index += Len;
                break;
            }
        } else {
            Chn = ch & 0xF;
            Cmd = ch >> 4;

            switch (Cmd) {
            case 0x8: // Note Off
                n1 = get_byte(Tr);
                n2 = get_byte(Tr);

                NoteOff(Chn, n1, n2);
                break;

            case 0x9: // Note On
                n1 = get_byte(Tr);
                n2 = get_byte(Tr);

                // Clamp to max allowed speed
                if (n2 > 127)
                    n2 = 127;

                // If speed is zero, it's like NoteOff
                if (n2 == 0)
                    NoteOff(Chn, n1, n2);
                else
                    NoteOn(Chn, n1, n2);
                break;

            case 0xA: // Key aftertouch
                n1 = get_byte(Tr);
                n2 = get_byte(Tr);

                KAfter(Chn, n1, n2);
                break;

            case 0xB: // Control change
                n1 = get_byte(Tr);
                n2 = get_byte(Tr);

                CChange(Chn, n1, n2);
                break;

            case 0xC: // Patch change
                n1 = get_byte(Tr);

                PChange(Chn, n1);
                break;

            case 0xD: // Channel aftertouch
                n1 = get_byte(Tr);

                CAfter(Chn, n1);
                break;

            case 0xE: // Pitch change
                n1 = get_byte(Tr);
                n2 = get_byte(Tr);

                WChange(Chn, n1, n2);
                break;

            case 0xF: // System Messages (UNUSED)
                break;
            }
        }
    }

	void init(void) {
	}

    void free_tracks(void)
    {
        unsigned int n;

        for (n = 0; n < MAX_TRACKS; n++)
        {
            if (Track[n].Data != NULL)
            {
                delete[] Track[n].Data;
                Track[n].Data = NULL;
            }
        }
    }

	bool load(File *f) {
        int n;

        Header.File.MThd   = f->readUint32BE();
        Header.File.Size   = f->readUint32BE();
        Header.File.Format = f->readUint16BE();
        Header.File.Tracks = f->readUint16BE();
        Header.File.Ticks  = f->readUint16BE();

        // Check if it's a MIDI file
        if (Header.File.MThd != SIGN_MThd ||
            Header.File.Size != 6 ||
            (Header.File.Format != 0 && Header.File.Format != 1))
            return 0;

        if (Header.File.Format == 0)
            Header.File.Tracks = 1;

        Tracks    = Header.File.Tracks;
        QNoteTime = Header.File.Ticks;

        // Read the tracks
        for (n = 0; n < Tracks; n++)
        {
            Header.Track.MTrk = f->readUint32BE();
            Header.Track.Size = f->readUint32BE();

            if (Header.Track.MTrk != SIGN_MTrk)
            {
                free_tracks();

                return 0;
            }

            Track[n].Size = Header.Track.Size;
            Track[n].Data = new uint8_t[Header.Track.Size];

            f->read(Track[n].Data, Header.Track.Size);

            Track[n].LastTime = 0;
            Track[n].Index    = 0;
            Track[n].LastCmd  = 0xFF;

            Track[n].DeltaTime = get_RLE(Track+n) * BaseTime / 1000;
        }

        TimeTicks = 0;
        LastTicks = _stub->getTimeStamp();

        BaseTime  = 500000 / QNoteTime;

		return 1;
	}

	void unload() {
        free_tracks();

        for (int n = 0; n < 16; n++)
        {
            CChange(n, 120, 0);
        }
	}

	bool update(void) {
        MidiTrack *Tr;
        uint32_t ms;
        int n, playing = 0;

        for (n = 0, Tr = Track; n < Tracks; n++, Tr++)
        {
            if (Tr->Index < Tr->Size || Tr->DeltaTime > TimeTicks)
                playing = 1;
        }

        if (!playing)
            return false;

        ms = _stub->getTimeStamp();

        TimeTicks += (ms - LastTicks);
        LastTicks = ms;

        for (n = 0, Tr = Track; n < Tracks; n++, Tr++)
        {
            while (Tr->Index < Tr->Size && Tr->DeltaTime < TimeTicks)
            {
                parse(n);

                Tr->DeltaTime += get_RLE(Tr) * BaseTime / 1000;
            }
        }

		return true;
	}
};

MidPlayer::MidPlayer(SystemStub *stub, Mixer *mixer, FileSystem *fs)
	: _playing(false), _mix(mixer), _fs(fs) {
	_impl = new MidPlayer_impl;

    _impl->_stub = stub;
}

MidPlayer::~MidPlayer() {
    // This should be called outside this source code
    WinMidi_Close();

	delete _impl;
}

void MidPlayer::play(int num) {
	if (num >= _midiFilesCount)
        return;

	File f;
	for (uint8_t i = 0; i < _midiFilesCount; ++i) {
		if (f.open(_midiFiles[num][i], "rb", _fs)) {
			_impl->init();
			if (_impl->load(&f)) {
				_impl->_repeatIntro = (num == 0);
				_mix->setPremixHook(mixCallback, _impl);
				_playing = true;
			}
			break;
		}
	}
}

void MidPlayer::stop() {
	if (_playing) {
		_mix->setPremixHook(0, 0);
		_impl->unload();
		_playing = false;
	}
}

bool MidPlayer::mixCallback(void *param, int16_t *buf, int len) {
	return ((MidPlayer_impl *)param)->update();
}
