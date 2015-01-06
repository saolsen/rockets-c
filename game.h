//#include <stdlib.h>
#ifndef _game_h
#define _game_h
#include <stdbool.h>

typedef struct {
    // position
    int mouse_x;
    int mouse_y;
    // if it's a normal click
    bool click;
    // todo(stephen): is there a better way to handle a drag?
    bool is_dragging;
    bool end_dragging;
    // todo(stephen): flesh this out with useful shit.
} ControllerState;

//todo(stephen): you may need to add a handler for the keyboard and mouse
//               movement callbacks if sdl is laggy polling the input.

struct game_api {
    /* Returns the initial state for the game. */
    void* (*init)();
    /* Passed the game state, controller input and the seconds elapsed */
    void (*update_and_render)(void* game_state,
                              ControllerState controller_state,
                              float dt);
};

extern const struct game_api GAME_API;

#endif
