CFLAGS=`pkg-config --cflags sdl2` -std=c99 -g -Wall -O0
LDFLAGS=`pkg-config --libs sdl2`

game: sdl_platform.c
	clang $(CFLAGS) $(LDFLAGS) -o game sdl_platform.c

clean:
	rm -rf *o *.dSYM game
