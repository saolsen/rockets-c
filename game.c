#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <sys/stat.h>

#include <OpenGL/gl3.h>

#include "game.h"

#define TILE_MAP_WIDTH 16
#define TILE_MAP_HEIGHT 9

// Not really good, should return an int and do error checking the c way.
char* read_whole_file(char* filename)
{
    FILE *fp = fopen(filename, "r");
    char* buffer = 0;
    long length;

    /* struct stat sb; */
    /* FILE *src = NULL; */

    /* stat(filename, &sb); */
    
    if(fp) {
        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        buffer = malloc(length + 1);
        if(buffer) {
            length = fread(buffer, 1, length, fp);
            buffer[length] = 0;
        }
        fclose(fp);
    }

    return buffer;
}


/* Game structs */
typedef struct Point {
    float x;
    float y;
} Point;

// todo(stephen): size of this is 56, not exactly a good number...
typedef struct Node {
    enum { ROOT, THRUSTER, PREDICATE, SIGNAL, CONSTANT, GATE } type;
    int id;
    Point position;
    union {
        struct {
            struct Node *lhs;
            struct Node *rhs; } input;
        struct Node             *parent;
    };
    union {
        enum { BP, BS, SP, SS, BOOST }       thruster;
        enum { LT, GT, LEQT, GEQT, EQ, NEQ } predicate;
        enum { POS_X }                       signal;
        int                                  constant;
        enum { AND, OR, NOT }                gate;
    };
} Node;

typedef struct {
    bool is_initialized;
    int last_id;
    Node* nodes[10];

    // Where should this opengl data really go?
    
    
} GameState;

// todo(stephen): use Point instead of Vector or even have both!

/* static Vector normalize(Vector v) { */
/*     float mag = sqrt(v.x * v.x + v.y * v.y);; */
/*     v.x /= mag; */
/*     v.y /= mag; */
/*     return v; */
/* } */

/* static Vector smul(Vector v, float n) { */
/*     v.x *= n; */
/*     v.y *= n; */
/*     return v; */
/* } */

/* static Vector add(Vector a, Vector b) { */
/*     a.x += b.x; */
/*     a.y += b.y; */
/*     return a; */
/* } */

/* static Vector subtract(Vector a, Vector b) { */
/*     a.x -= b.x; */
/*     a.y -= b.y; */
/*     return a; */
/* } */

/* static int round_to_int(float n) { */
/*     if (n >= 0.0) { */
/*         return (int)(n + 0.5); */
/*     } else { */
/*         return (int)(n - 0.5); */
/*     } */
/* } */

/* static uint32_t round_to_uint(float n) { */
/*     if (n >= 0.0) { */
/*         return (uint32_t)(n + 0.5); */
/*     } else { */
/*         return (uint32_t)(n - 0.5); */
/*     } */
/* } */


/* // todo(stephen): make draw_rectangle not overflow rows */
/* // todo(stephen): sub-pixel rendering so the tiger looks good. */
/* static void draw_rectangle(PixelBuffer pixel_buffer, */
/*                            float top_left_x, */
/*                            float top_left_y, */
/*                            float width, */
/*                            float height, */
/*                            float red, float green, float blue) */
/* { */
/*     assert(red >=0.0 && red <= 1.0); */
/*     assert(green >=0.0 && green <= 1.0); */
/*     assert(blue >=0.0 && blue <= 1.0); */
    
/*     int start_y = round_to_int(top_left_y); */
/*     int start_x = round_to_int(top_left_x); */

/*     uint32_t value = ((round_to_uint(red * 255.0) << 16) + */
/*                       (round_to_uint(green * 255.0) << 8) + */
/*                       (round_to_uint(blue * 255.0))); */

/*     for (int row = start_y; */
/*          row < start_y + round_to_int(height); */
/*          row++) { */

/*         for (int column = start_x; */
/*              column < start_x + round_to_int(width); */
/*              column ++) { */

/*             int pixel = (row * pixel_buffer.width) + column; */
/*             if (pixel > 0 && */
/*                 pixel < pixel_buffer.width * pixel_buffer.height) { */
                
/*                 pixel_buffer.buffer[pixel] = value; */
/*             } */
/*         } */
/*     } */
/* } */


/* void draw_node(PixelBuffer pixel_buffer, Node node) { */

/*     /\* float r, g, b; *\/ */
/*     /\* int width, height; *\/ */

/*     /\* switch (node->type) { *\/ */
/*     /\* case PREDICATE: *\/ */
/*     /\*     r = 1.0; *\/ */
/*     /\*     g = 0.0; *\/ */
/*     /\*     b = 0.0; *\/ */
/*     /\*     width = 30; *\/ */
/*     /\*     height = 30; *\/ */
/*     /\*     break; *\/ */
/*     /\* } *\/ */
    
/*     draw_rectangle(pixel_buffer, */
/*                    node.position.x, node.position.y, */
/*                    10.0, 10.0, */
/*                    1.0, 1.0, 1.0); */
/* } */

int next_id(int *last_id) {
    *last_id += 1;
    return *last_id;
}

void* game_setup()
{
    printf("setup setting last_id to 100");
    GameState *gamestate = malloc(sizeof(gamestate));
    gamestate->last_id = 100;
    return gamestate;
}

void game_update_and_render(void *gamestate,
                            ControllerState controller_state,
                            float dt)
{
    GameState *state = (GameState*)gamestate;

    if (!state->is_initialized) {
        printf("value of last_id is %d", state->last_id);
        state->last_id = 0;
        state->is_initialized = true;
    }

    /* clear window to black */
    glClearColor(1,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT);

    Node thruster = {.type = THRUSTER,
                     .id = next_id(&state->last_id),
                     .position = {.x = 110.0, .y = 100.0},
                     .thruster = BOOST };
}

const struct game_api GAME_API = {
    .init = game_setup,
    .update_and_render = game_update_and_render
};
