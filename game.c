#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

#include "game.h"

#define TILE_MAP_WIDTH 16
#define TILE_MAP_HEIGHT 9

// todo(stephen): use Point instead of Vector or even have both!

static Vector normalize(Vector v) {
    float mag = sqrt(v.x * v.x + v.y * v.y);;
    v.x /= mag;
    v.y /= mag;
    return v;
}

static Vector smul(Vector v, float n) {
    v.x *= n;
    v.y *= n;
    return v;
}

static Vector add(Vector a, Vector b) {
    a.x += b.x;
    a.y += b.y;
    return a;
}

static Vector subtract(Vector a, Vector b) {
    a.x -= b.x;
    a.y -= b.y;
    return a;
}

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


// todo(stephen): make draw_rectangle not overflow rows
// todo(stephen): sub-pixel rendering so the tiger looks good.
static void draw_rectangle(PixelBuffer pixel_buffer,
                           float top_left_x,
                           float top_left_y,
                           float width,
                           float height,
                           float red, float green, float blue)
{
    assert(red >=0.0 && red <= 1.0);
    assert(green >=0.0 && green <= 1.0);
    assert(blue >=0.0 && blue <= 1.0);
    
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
            if (pixel > 0 &&
                pixel < pixel_buffer.width * pixel_buffer.height) {
                
                pixel_buffer.buffer[pixel] = value;
            }
        }
    }
}


static void draw_tiger(PixelBuffer pixel_buffer,
                       float x, float y)
{
    /* orange */
    float r = 247.0 / 255;
    float g = 147.0 / 255;
    float b = 29.0 / 255;

    /* draw body */
    draw_rectangle(pixel_buffer, -16.0 + x, -20.0 + y, 32.0, 12.0, r, g, b);
    /* draw legs */
    draw_rectangle(pixel_buffer, -16.0 + x, -8.0 + y, 4.0, 8.0, r, g, b);
    draw_rectangle(pixel_buffer, 12.0 + x, -8.0 + y, 4.0, 8.0, r, g, b);
    /* todo(stephen): flip head and tail based on direction */
    /* draw head */
    draw_rectangle(pixel_buffer, -24.0 + x, -28.0 + y, 12.0, 12.0, r, g, b);
    /* draw tail */
    draw_rectangle(pixel_buffer, 16.0 + x, -20.0 + y, 12.0, 4.0, r, g, b);
}


void game_update_and_render(PixelBuffer pixel_buffer,
                            void *gamestate,
                            float dt)
{
    GameState *state = (GameState*)gamestate;
    /* first test, just do a tile map and then test some A* pathfinding
     * fun tests
     */

    // todo(stephen): I need a gamestate to do A* pathfinding.
    //                also I know some stuff will change with the
    //                way I do tile maps, like how I wont necessesarily
    //                even use one. Definately not (1 per screen)

    if (!state->is_initialized) {
        state->pos.x = 75.0;
        state->pos.y = 75.0;
        state->target.x = 375;
        state->target.y = 25;
        state->is_initialized = true;
    }

    int tile_map[TILE_MAP_HEIGHT][TILE_MAP_WIDTH] =
        {{1, 0, 0, 0,  0, 0, 1, 0,  0, 0, 0, 0,  0, 0, 0, 0},
         {0, 0, 0, 0,  0, 0, 1, 1,  0, 0, 0, 0,  0, 0, 0, 0},
         {1, 0, 0, 0,  0, 0, 0, 1,  1, 1, 0, 0,  1, 0, 0, 0},
         {1, 0, 0, 0,  0, 0, 0, 0,  0, 1, 1, 0,  0, 0, 0, 0},
         {1, 0, 0, 0,  0, 0, 0, 1,  0, 0, 1, 0,  0, 0, 0, 0},
         {1, 0, 0, 0,  0, 0, 0, 1,  0, 0, 1, 1,  0, 0, 0, 0},
         {1, 0, 0, 0,  0, 0, 0, 1,  0, 0, 1, 0,  0, 0, 0, 0},
         {1, 0, 0, 0,  0, 0, 0, 1,  0, 0, 1, 0,  0, 0, 0, 0},
         {1, 1, 1, 1,  1, 1, 1, 1,  0, 0, 0, 0,  0, 0, 0, 0}};

    float tile_width = 50.0f;
    float tile_height = 50.0f;
    
    for (int i = 0; i < TILE_MAP_HEIGHT; i++) {
        for (int j = 0; j < TILE_MAP_WIDTH; j++) {

            int is_filled = tile_map[i][j];
            float gray_val = 0.25;

            if (is_filled == 1) {
                gray_val = 0.75;
            }
            
            draw_rectangle(pixel_buffer,
                           j * tile_width,
                           i * tile_height,
                           tile_width,
                           tile_height,
                           gray_val, gray_val, gray_val);
        }
    }

    // todo(stephen): collision detection.
    // todo(stephen): pathfinding
    // move tiger towards target point.
    // this is basically impossible without vectors.
    /* float speed = 20.0; */
    /* if (state->pos.x != state->target.x) { */
    /*     Vector direction = normalize(subtract(state->target, state->pos)); */
    /*     state->pos = add(state->pos, smul(direction, dt * speed)); */
    /* } */

    // move tiger based on player input.
    


    draw_tiger(pixel_buffer, state->pos.x, state->pos.y);
}
