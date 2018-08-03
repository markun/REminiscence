#ifndef MID_PLAYER_H__
#define MID_PLAYER_H__

#include "intern.h"

// Called when 'Note On' event is received
typedef void (*MidiPlay_NoteOn) (int, int, int);

// Called when 'Note Off' event is received
typedef void (*MidiPlay_NoteOff)(int, int, int);

// Called when 'Key aftertouch' event is received
typedef void (*MidiPlay_KAfter) (int, int, int);

// Called when 'Control change' event is received
typedef void (*MidiPlay_CChange)(int, int, int);

// Called when 'Path change' event is received
typedef void (*MidiPlay_PChange)(int, int);

// Called when 'Channel aftertouch' event is received
typedef void (*MidiPlay_CAfter) (int, int);

// Called when 'Pitch change' event is received
typedef void (*MidiPlay_WChange)(int, int, int);

struct FileSystem;
struct Mixer;
struct MidPlayer_impl;
struct SystemStub;

struct MidPlayer {

	static const char *_midiFiles[][1];
	static const int _midiFilesCount;

	bool _playing;
	Mixer *_mix;
        FileSystem *_fs;
	MidPlayer_impl *_impl;

    MidPlayer(SystemStub *stub, Mixer *mixer, FileSystem *fs);
	~MidPlayer();

	void play(int num);
	void stop();

	static bool mixCallback(void *param, int16_t *buf, int len);
};

#endif // MID_PLAYER_H__
