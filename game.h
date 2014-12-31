//#include <stdlib.h>
#include <stdbool.h>

// Shared structs
typedef struct {
    uint32_t *buffer;
    int width;
    int height;
} PixelBuffer;

// Game interface
typedef void (*UpdateFunction)(PixelBuffer, void*, float);
//todo(stephen): how do I declare that game_update_and_render
// has type UpdateFunction.
void game_update_and_render(PixelBuffer pixel_buffer,
                            void* state,
                            float dt);

// Game structs
typedef struct {
    float pos_x;
    float pos_y;
    bool is_initialized;
} GameState;
