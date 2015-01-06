#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <sys/stat.h>

#include <OpenGL/gl3.h>

#include "game.h"

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


typedef struct {
    // opengl shit
    
} GameState;


void* game_setup()
{
    GameState *gamestate = malloc(sizeof(gamestate));
    
    return gamestate;
}

void game_update_and_render(void *gamestate,
                            ControllerState controller_state,
                            float dt)
{
    GameState *state = (GameState*)gamestate;

    /* clear window to black */
    glClearColor(1,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT);


}

const struct game_api GAME_API = {
    .init = game_setup,
    .update_and_render = game_update_and_render
};
