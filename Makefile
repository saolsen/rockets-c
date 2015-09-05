# This only works on osx right now, @TODO get it building for linux (maybe
# windows) and ios.
# Only non git reqirement is sdl2, it must be installed before running this
# makefile.
# `brew install sdl2`

OPENGL_LIB= -framework OpenGL
NANOVG_I= -Inanovg/src
NANOVG_LD= -Lnanovg/build -lnanovg

# -Weverything -pedantic -Werror
CFLAGS= $(NANOVG_I) -std=c11 -g -Wall -O0 -fPIC `pkg-config --cflags sdl2`
LDFLAGS= $(NANOVG_LD) $(OPENGL_LIB) `pkg-config --libs sdl2`

all: libgame.dylib rockets

libgame.dylib: rockets.c rockets.h rockets_platform.h rockets_*.h rockets_*.c
	clang -dynamiclib -undefined dynamic_lookup $(CFLAGS) -o libgame.dylib rockets.c

rockets: rockets_platform.c rockets_platform.h
	clang $(CFLAGS) $(LDFLAGS) -o rockets rockets_platform.c

clean:
	rm -r rockets libgame.dylib*

check-syntax:
	clang -o /dev/null $(CFLAGS) -S ${CHK_SOURCES}

tags:
	etags ./*.c ./*.h nanovg/src/nanovg.h
