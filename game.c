#include <stdlib.h>
#include <stdio.h>

#include "game.h"

static int round_to_int(float n) {
    if (n >= 0.0) {
        return (int)(n + 0.5);
    } else {
        return (int)(n - 0.5);
    }
}

static uint32_t round_to_uint(float n) {
    if (n >= 0.0) {
        return (uint32_t)(n + 0.5);
    } else {
        return (uint32_t)(n - 0.5);
    }
}


static void draw_rectangle(PixelBuffer pixel_buffer,
                           float top_left_x,
                           float top_left_y,
                           float width,
                           float height,
                           float red, float green, float blue)
{
    int start_y = round_to_int(top_left_y);
    int start_x = round_to_int(top_left_x);

    uint32_t value = ((round_to_uint(red * 255.0) << 16) +
                      (round_to_uint(green * 255.0) << 8) +
                      (round_to_uint(blue * 255.0)));

    for (int row = start_y;
         row < start_y + round_to_int(height);
         row++) {
        for (int column = start_x;
             column < start_x + round_to_int(width);
             column ++) {

            int pixel = (row * pixel_buffer.width) + column;

            pixel_buffer.buffer[pixel] = value;
        }
    }
}

void game_update_and_render(PixelBuffer pixel_buffer)
{
    // screen_width aka pitch should go in a struct with the pixel buffer
    printf("hitting here at least\n");
    draw_rectangle(pixel_buffer,
                   100.0f, 200.0f, 100.0f, 300.0f,
                   1.0f, 0.0f, 0.0f);
}
