//#include <stdlib.h>
#ifndef _game_h
#define _game_h
#include <stdbool.h>

/* Shared structs */
// had just pixel_buffer before but I need access to the renderer and will be making textures
// and shit now because I'm using the sdl2 rendering. bleh
// some day maybe write my own 
typedef struct {
    uint32_t *buffer;
    int width;
    int height;
} PixelBuffer;

typedef struct {
    // position
    int mouse_x;
    int mouse_y;
    // if it's a normal click
    bool click;
    // todo(stephen): is there a better way to handle a drag?
    bool is_dragging;
    bool end_dragging;
} ControllerState;

/* Game interface */
typedef void (*UpdateFunction)(PixelBuffer, void*, ControllerState, float);
//todo(stephen): how do I declare that game_update_and_render
// has type UpdateFunction.
void game_update_and_render(PixelBuffer pixel_buffer,
                            void* gamestate,
                            ControllerState controller_state,
                            float dt);

#endif
