BIN_PREFIX=/usr/i686-w64-mingw32/sys-root/mingw/bin
CXX=i686-w64-mingw32-c++

SDL_CFLAGS   := `$(BIN_PREFIX)/sdl2-config --cflags`
SDL_LIBS     := `$(BIN_PREFIX)/sdl2-config --libs`

DL_LIBS      := #-ldl
MODPLUG_LIBS := #-lmodplug
TREMOR_LIBS  := #-lvorbisidec -logg
ZLIB_LIBS    := -lz
MIDI_LIBS	 := -lwinmm

CXXFLAGS += -Wall -MMD $(SDL_CFLAGS) -DUSE_ZLIB #-DUSE_TREMOR -DUSE_MODPLUG

SRCS = collision.cpp cutscene.cpp dynlib.cpp file.cpp fs.cpp game.cpp graphics.cpp main.cpp menu.cpp \
	mixer.cpp mod_player.cpp ogg_player.cpp piege.cpp resource.cpp resource_aba.cpp \
	scaler.cpp screenshot.cpp seq_player.cpp mid_player.cpp \
	sfx_player.cpp staticres.cpp systemstub_sdl.cpp unpack.cpp util.cpp video.cpp

OBJS = $(SRCS:.cpp=.o)
DEPS = $(SRCS:.cpp=.d)

LIBS = $(SDL_LIBS) $(DL_LIBS) $(MODPLUG_LIBS) $(TREMOR_LIBS) $(ZLIB_LIBS) $(MIDI_LIBS)

rs: $(OBJS)
	$(CXX) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	rm -f *.o *.d

-include $(DEPS)
