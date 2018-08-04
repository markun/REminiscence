#ifndef MID_PLAYER_H__
#define MID_PLAYER_H__

#include "intern.h"

struct FileSystem;
struct Mixer;
struct MidPlayer_impl;
struct SystemStub;
struct MidiDevice;

struct MidPlayer {

	static const char *_midiFiles[][1];
	static const int _midiFilesCount;

	bool _playing;
	Mixer *_mix;
        FileSystem *_fs;
	MidPlayer_impl *_impl;

    MidPlayer(MidiDevice *device, SystemStub *stub, Mixer *mixer, FileSystem *fs);
	~MidPlayer();

	void play(int num);
	void stop();

	static bool mixCallback(void *param, int16_t *buf, int len);
};

#endif // MID_PLAYER_H__
