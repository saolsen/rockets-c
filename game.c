#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>
#include <sys/stat.h>

/* #include <OpenGL/gl3.h> */

#include "gameguy.h"

// Not really good, should return an int and do error checking the c way.
const char* read_whole_file(const char* filename)
{
    FILE *fp = fopen(filename, "r");
    char* buffer = 0;
    long length;

    /* struct stat sb; */
    /* FILE *src = NULL; */

    /* stat(filename, &sb); */
    
    if (fp) {
        fseek(fp, 0, SEEK_END);
        length = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        buffer = malloc(length + 1);
        if (buffer) {
            length = fread(buffer, 1, length, fp);
            buffer[length] = 0;
        }
        fclose(fp);
    }

    return buffer;
}

const float vertex_data[] = {
    0.75f, 0.75f, 0.0f, 1.0f,
    0.75f, -0.75f, 0.0f, 1.0f,
    -0.75f, -0.75f, 0.0f, 1.0f,
};


typedef struct {
    // opengl shit
    GLuint positionBufferObject;
    GLuint theProgram;
    GLuint vao;
} GameState;


static void* game_setup()
{
    GameState *state = malloc(sizeof(GameState));

    // Initialize Program
    // todo(stephen): Figure out shader loading, right now path convention.
    const char* vertex_src = read_whole_file("shaders/test_vert.glsl");
    GLuint v_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(v_shader, 1, &vertex_src, NULL);
    glCompileShader(v_shader);

    const char* fragment_src = read_whole_file("shaders/test_frag.glsl");
    GLuint f_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(f_shader, 1, &fragment_src, NULL);
    glCompileShader(f_shader);

    GLuint program = glCreateProgram();
    glAttachShader(program, v_shader);
    glAttachShader(program, f_shader);
    glLinkProgram(program);
    state->theProgram = program;

    //todo(stephen): check for compilation errors.

    // Initialize Vertex Buffer
    GLuint buffer_object;
    glGenBuffers(1, &buffer_object);
    glBindBuffer(GL_ARRAY_BUFFER, buffer_object);
    glBufferData(GL_ARRAY_BUFFER,
                 sizeof(vertex_data),
                 vertex_data,
                 GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    state->positionBufferObject = buffer_object;

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    state->vao = vao;
    
    return state;
}

static void game_update_and_render(void *gamestate,
                                   gg_Input input,
                                   float dt)
{
    GameState *state = (GameState*)gamestate;

    /* clear window to black */
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glUseProgram(state->theProgram);

    glBindBuffer(GL_ARRAY_BUFFER, state->positionBufferObject);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, 0);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    glDisableVertexAttribArray(0);
    glUseProgram(0);
}

const gg_Game gg_game_api = {
    .init = game_setup,
    .update_and_render = game_update_and_render
};
