# This only works on osx right now, @TODO get it building for linux (maybe
# windows) and ios.
# Only non git reqirement is sdl2, it must be installed before running this
# makefile.
# `brew install sdl2`

OPENGL_LIB= -framework OpenGL
NANOVG_I= -Inanovg/src
NANOVG_LD= -Lnanovg/build -lnanovg

# -Weverything -pedantic -Werror
CXXFLAGS= $(NANOVG_I) -std=c++11 -g -Wall -O0 -fPIC `pkg-config --cflags sdl2`
LDFLAGS= $(NANOVG_LD) $(OPENGL_LIB) `pkg-config --libs sdl2`

all: libgame.dylib rockets

libgame.dylib: game.cpp game.hpp gameguy.hpp
	c++ -dynamiclib -undefined dynamic_lookup $(CFLAGS) -o libgame.dylib game.cpp

rockets: gameguy.cpp gameguy.hpp
	c++ $(CFLAGS) $(LDFLAGS) -o rockets gameguy.cpp

clean:
	rm -r rockets* libgame*

check-syntax:
	c++ -o /dev/null $(CFLAGS) -S ${CHK_SOURCES}

tags:
	etags gameguy.cpp gameguy.hpp game.cpp nanovg/src/nanovg.h
