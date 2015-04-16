# This only works on osx right now, TODO get it building for linux (maybe
# windows) and ios.
# Only non git reqirement is sdl2, it must be installed before running this
# makefile.
# `brew install sdl2`

OPENGL_LIB= -framework OpenGL
NANOVG_I= -Inanovg/src
NANOVG_LD= -Lnanovg/build -lnanovg

CFLAGS= $(NANOVG_I) -std=c11 -g -Wall -O0 -fPIC `pkg-config --cflags sdl2`
LDFLAGS= $(NANOVG_LD) $(OPENGL_LIB) `pkg-config --libs sdl2`

all: libgame.dylib rockets

libgame.dylib: game.c
	clang -dynamiclib -undefined dynamic_lookup $(CFLAGS) -o libgame.dylib game.c

rockets: gameguy.c
	clang $(CFLAGS) $(LDFLAGS) -o rockets gameguy.c

clean:
	rm -r rockets* libgame*

check-syntax:
	clang -o /dev/null $(CFLAGS) -S ${CHK_SOURCES}
