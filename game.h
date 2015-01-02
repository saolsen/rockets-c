//#include <stdlib.h>
#include <stdbool.h>

/* Shared structs */
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

/* Game structs. These could just go in game.c */
typedef struct {
    float x;
    float y;
} Vector;

typedef struct {
    Vector pos;
    bool dragging_tiger;
    bool is_initialized;
} GameState;
