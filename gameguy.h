#ifndef _gameguy_h
#define _gameguy_h

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <sys/stat.h>

// @TODO: Make this cross platform.
#include <OpenGL/gl3.h>

#include "nanovg.h"

// Libraries being used
// nanovg
// stb stuff, not sure which yet

#define SCREEN_WIDTH  1280
#define SCREEN_HEIGHT 720

// Library Functions

// @TODO: add printing, debugging, and profiling macros here.
#define log_info(M, ...) fprintf(stderr, "[INFO] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)
// @TODO: Have errors log in red.
#define log_error(M, ...) fprintf(stderr, "[ERROR] (%s:%d) " M "\n", __FILE__, __LINE__, ##__VA_ARGS__)

// @TODO: IF DEBUG
/* extern void* gg_Debug_Memory; */

// Interface with Game.
typedef struct {
    int mouse_x;
    int mouse_y;
    int mouse_xrel;
    int mouse_yrel;
    bool mouse_motion;
    bool click;
    bool start_dragging;
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
