MACHINE= $(shell uname -s)
GAMELIB= gamelib.so
BUILD_DIR?= bin
EXE_NAME?= game

ifeq ($(MACHINE),Darwin)
	OPENGL_LIB= -framework OpenGL
else
	OPENGL_LIB= -I/usr/lib64 -lGL -lGLEW
endif

CFLAGS= -Igameguy/include -std=c99 -g -Wall -O0

all: game lib

.PHONY: game
game:
	cd gameguy && $(MAKE) BUILD_DIR=../$(BUILD_DIR) EXE_NAME=$(EXE_NAME) gameguy

lib: game.c
	clang $(CFLAGS) $(OPENGL_LIB) -shared -o $(BUILD_DIR)/$(GAMELIB) game.c

clean:
	rm -rf *o *.dSYM $(BUILD_DIR)/*

check-syntax:
	clang -o /dev/null $(CFLAGS) -S ${CHK_SOURCES}
