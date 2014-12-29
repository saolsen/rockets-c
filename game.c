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

// todo(stephen): make draw_rectangle not crash on out of bounds areas.
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
    /* first test, just going to do a tile map and then test some A* pathfinding
     * fun tests
     */

    // todo(stephen): I need a gamestate to do A* pathfinding.
    //                also I know some stuff will change with the
    //                way I do tile maps, like how I wont necessesarily
    //                even use one. Definately not (1 per screen)
    const int tile_map_width = 18;
    const int tile_map_height = 10;

    int tile_map[10][18] =
        {{1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
         {0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
         {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0},
         {0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0},
         {0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0},
         {0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
         {0, 0, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0},
         {0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0},
         {0, 0, 1, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0},
         {0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0}};

    float tile_width = 50.0f;
    float tile_height = 50.0f;
    
    for (int i = 0; i < tile_map_height; i++) {
        for (int j = 0; j < tile_map_width; j++) {

            int is_filled = tile_map[i][j];
            float gray_val = 0.75;

            if (is_filled == 1) {
                gray_val = 0.25;
            }
            
            draw_rectangle(pixel_buffer,
                           j * tile_width,
                           i * tile_height,
                           tile_width,
                           tile_height,
                           gray_val, gray_val, gray_val);
        }
    }
}
