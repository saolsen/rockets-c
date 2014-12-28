//#include <stdlib.h>

// Shared structs
typedef struct {
    uint32_t *buffer;
    int width;
    int height;
} PixelBuffer;

// Game interface
typedef void (*UpdateFunction)(PixelBuffer);
//todo(stephen): how do I declare that game_update_and_render
// has type UpdateFunction.
void game_update_and_render(PixelBuffer pixel_buffer);

// Game structs
