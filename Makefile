CFLAGS=`pkg-config --cflags sdl2` -std=c99 -g -Wall -O0
LDFLAGS=`pkg-config --libs sdl2` -ldl

all: main libgame.so

main: sdl_platform.c game.h
	clang $(CFLAGS) $(LDFLAGS) -o game sdl_platform.c

libgame.so: game.c game.h
	clang $(CFLAGS) -shared $(LDFLAGS) -o libgame.so game.c

clean:
	rm -rf *o *.dSYM game

check-syntax:
	clang -o /dev/null $(CFLAGS) $(LDFLAGS) -S ${CHK_SOURCES}
