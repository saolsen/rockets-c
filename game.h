//#include <stdlib.h>
#include <stdbool.h>

// Shared structs
typedef struct {
    uint32_t *buffer;
    int width;
    int height;
} PixelBuffer;

// aparently floating point math is faster than ints
typedef struct {
    int mouse_x;
    int mouse_y;
    // buttons, press or isdown? casey's thing?
    // mouse position and clicks and drags.
} ControllerState;

// Game interface
typedef void (*UpdateFunction)(PixelBuffer, void*, float);
//todo(stephen): how do I declare that game_update_and_render
// has type UpdateFunction.
void game_update_and_render(PixelBuffer pixel_buffer,
                            void* gamestate,
                            float dt);

// Game structs
typedef struct {
    float x;
    float y;
} Vector;

typedef struct {
    Vector pos;
    Vector target;
    bool is_initialized;
} GameState;
