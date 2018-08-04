#include "file.h"
#include "mixer.h"
#include "systemstub.h"
#include "util.h"
#include "midi_device.h"

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

struct MidPlayer_impl {

    unsigned int QNoteTime;
    unsigned int BaseTime;

    unsigned int TimeTicks;
    unsigned int LastTicks;
    int Tracks;

    MidiTrack Track[MAX_TRACKS];

	bool _repeatIntro;

    union {
        MidiFileHeaderType  File;
        MidiTrackHeaderType Track;
    } Header;

    MidiDevice *_device;
	SystemStub *_stub;

	MidPlayer_impl(MidiDevice *device, SystemStub *stub) : _device(device), _stub(stub) {

        // Initialize pointer to tracks
        for (int n = 0; n < MAX_TRACKS; n++)
        {
            Track[n].Data = NULL;
        }
	}

	~MidPlayer_impl() {
        delete _device;
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

                _device->NoteOff(Chn, n1, n2);
                break;

            case 0x9: // Note On
                n1 = get_byte(Tr);
                n2 = get_byte(Tr);

                // Clamp to max allowed speed
                if (n2 > 127)
                    n2 = 127;

                // If speed is zero, it's like NoteOff
                if (n2 == 0)
                    _device->NoteOff(Chn, n1, n2);
                else
                    _device->NoteOn(Chn, n1, n2);
                break;

            case 0xA: // Key aftertouch
                n1 = get_byte(Tr);
                n2 = get_byte(Tr);

                _device->KAfter(Chn, n1, n2);
                break;

            case 0xB: // Control change
                n1 = get_byte(Tr);
                n2 = get_byte(Tr);

                _device->CChange(Chn, n1, n2);
                break;

            case 0xC: // Patch change
                n1 = get_byte(Tr);

                _device->PChange(Chn, n1);
                break;

            case 0xD: // Channel aftertouch
                n1 = get_byte(Tr);

                _device->CAfter(Chn, n1);
                break;

            case 0xE: // Pitch change
                n1 = get_byte(Tr);
                n2 = get_byte(Tr);

                _device->WChange(Chn, n1, n2);
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
            _device->CChange(n, 120, 0);
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

MidPlayer::MidPlayer(MidiDevice *device, SystemStub *stub, Mixer *mixer, FileSystem *fs)
	: _playing(false), _mix(mixer), _fs(fs) {
	_impl = new MidPlayer_impl(device, stub);

    _impl->_stub = stub;
}

MidPlayer::~MidPlayer() {
    // This should be called outside this source code

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
