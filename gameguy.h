#ifndef _gameguy_h
#define _gameguy_h

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <sys/stat.h>

// todo(stephen): Make this cross platform.
#include <OpenGL/gl3.h>

#include "nanovg.h"

// Libraries being used
// nanovg
// stb stuff, not sure which yet
// TODO(stephen): Cmake stuff.
// Include all those here, only include this file from game.c
// Set up the nanovg context as part of gameguy because it will be different on different platforms.

#define SCREEN_WIDTH  2048
#define SCREEN_HEIGHT 1536

// Library Functions

// todo(stephen): add printing, debugging, and profiling macros here.
#define log_info(M, ...) fprintf(stderr, "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
// todo(stephen): Have errors log in red.
#define log_error(M, ...) fprintf(stderr, "[ERROR] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

// Opengl Helpers
// todo(stephen): This returns a buffer that has been malloc's so it needs to be
//                freed outside of this scope. How do I deal with that, is it
//                just about documentation? Do I have the user pass in a buffer?
char*
gg_read_whole_file(const char* filename)
{
    FILE *fp = fopen(filename, "r");
    char* buffer = 0;
    long length;

    if (fp) {
        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        buffer = malloc(length + 1);
        if (buffer) {
            length = fread(buffer, 1, length, fp);
            buffer[length] = 0;
        }
        fclose(fp);
    }

    return buffer;
}

// Interface with Game.

typedef struct {
    int mouse_x;
    int mouse_y;
    bool click;
    bool is_dragging;
    bool end_dragging;
} gg_Input;


typedef struct {
    void* (*init)(NVGcontext* vg);
    void (*update_and_render)(void* game_state,
                              NVGcontext* vg,
                              gg_Input input,
                              float dt);
} gg_Game;

extern const gg_Game gg_game_api;

#endif
