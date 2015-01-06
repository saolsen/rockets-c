// todo(stephen): Need to set up glew for linux and windows support.
#include <SDL2/SDL.h>
#include <OpenGL/gl3.h>

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <time.h>

#include "game.h"

// 2048‑by‑1536 is the real resolution
// todo(stephen): figure out if I need to draw to that or if I can get a real
//                osx high dpi context.
#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 768

// todo(stephen): provide a way to statically link this for release builds.
// todo(stephen): pass this in on the command line for which game to load.
#define GAME_LIBRARY "./libgame.so"
/* #define GAME_FUNCTION "game_update_and_render" */
/* #define SETUP_FUNCTION "game_setup" */

/* void *library_handle; */
/* /\* void (*game_update_and_render_fn)(uint32_t*, int); *\/ */
/* UpdateFunction game_update_and_render_fn; */
/* SetupFunction game_setup_fn; */

typedef struct Game {
    void *handle;
    ino_t id;
    struct game_api api;
} Game;

/* if (library_handle != NULL) { */
/*     dlclose(library_handle); */
/* } */
      
/* library_handle = dlopen(GAME_LIBRARY, RTLD_NOW); */
/* if (!library_handle) { */
/*     printf("Error loading library %s\n", dlerror()); */
/* } */

/* game_setup_fn = dlsym(library_handle, SETUP_FUNCTION); */

/* gamestate = game_setup_fn(); */


/* int reload_game(Game *game) { */
    
/* } */

float sdl_get_seconds_elapsed(uint64_t old_counter, uint64_t current_counter)
{
    return ((float)(current_counter - old_counter) /
            (float)SDL_GetPerformanceFrequency());
}

bool sdl_handle_event(SDL_Event *event, int *other_events_this_tick,
                      ControllerState *controller_state)
{
    bool should_quit = false;

    switch(event->type) {
    case SDL_QUIT:
        should_quit = true;
        break;

        // Going to use mouse events for now.
        // todo(stephen): use touch events when ready to port to ipad.
        // todo(stephen): figure out how to just do a click.
        // might want zooming
    case SDL_MOUSEBUTTONDOWN:
        /* printf("mouse down\n"); */
        controller_state->click = true;
        controller_state->is_dragging = true;
        break;

    case SDL_MOUSEBUTTONUP:
        /* printf("mouse up\n"); */
        controller_state->is_dragging = false;
        controller_state->end_dragging = true;
        break;
        // todo(stephen): there is a bug with window resizing here. I need the
        // position to match the logical screen size, not the real one.
    case SDL_MOUSEMOTION:
        controller_state->mouse_x = event->motion.x;
        controller_state->mouse_y = event->motion.y;
        break;

        //todo(stephen): SDL_SetWindowFullscreen when hitting f.
    case SDL_KEYDOWN: {
        printf("keypress\n");
        SDL_Keycode keycode = event->key.keysym.sym;

        if (keycode == SDLK_ESCAPE) {
            should_quit = true;
        }
    } break;

    default:
        //todo(stephen): print out that other events happened for debugging
        *other_events_this_tick+= 1;
        break;
    }

    return should_quit; 
}


int main(int argc, char* argv[])
{
    srand(time(NULL));
    // todo(stephen): get this from the video card and run at 60fps
    float game_update_hz = 60.0;
    float target_seconds_per_frame = 1.0 / game_update_hz;

    //todo(stephen): have a better error handling scheme that has all of these
    //               goto the end and write an error message instead of a bunch
    //               of different checks.
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Error initializing SDL: %s", SDL_GetError());
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    
    SDL_Window *window = SDL_CreateWindow("animal game",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SCREEN_WIDTH,
                                          SCREEN_HEIGHT,
                                          SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE);

    if (!window) {
      printf("Error creating window: %s", SDL_GetError());
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);

    //Use Vsync
    if( SDL_GL_SetSwapInterval( 1 ) < 0 ) {
        printf( "Warning: Unable to set VSync! SDL Error: %s\n", SDL_GetError() );
    }

    void *gamestate;
    ControllerState controller_state = {.mouse_x = 0,
                                        .mouse_y = 0,
                                        .click = false,
                                        .is_dragging = false,
                                        .end_dragging = false};

    // load game
    // todo(stephen): function for reloading.
    Game game;

    if (NULL != game.handle) {
        dlclose(game.handle);
    }

    game.handle = dlopen(GAME_LIBRARY, RTLD_NOW);
    if (!game.handle) {
        printf("Error loading library %s\n", dlerror());
    }

    struct game_api *api = dlsym(game.handle, "GAME_API");
    game.api = *api;

    // Don't really know how big this is going to be yet.

    bool running = true;
    uint64_t last_counter = SDL_GetPerformanceCounter();

    gamestate = game.api.init();

    while (running) {
        // reload library
        //todo(stephen): only reload if file has changed.
        if (NULL != game.handle) {
            dlclose(game.handle);
        }

        game.handle = dlopen(GAME_LIBRARY, RTLD_NOW);
        if (!game.handle) {
            printf("Error loading library %s\n", dlerror());
        }

        struct game_api *api = dlsym(game.handle, "GAME_API");
        game.api = *api;

        
        //todo(stephen): event handling
        SDL_Event event;
        int other_events_this_tick = 0;

        controller_state.end_dragging = false;;
        controller_state.click = false;
        
        while (SDL_PollEvent(&event)) {
            if (sdl_handle_event(&event,
                                 &other_events_this_tick,
                                 &controller_state)) {
                running = false;
            }
        }
        if (other_events_this_tick > 0) {
            /* printf("%d unhandled events this tick.\n",
               other_events_this_tick); */
        }

        /* run game tick */
        game.api.update_and_render(gamestate, controller_state,
                                   target_seconds_per_frame);

        //todo(stephen): decide if I can take out this delay code since I'm
        //               using vsync now.
        /* /\* delay the rest of the frame *\/ */
        /* int seconds_elapsed = */
        /*   sdl_get_seconds_elapsed(last_counter, */
        /*                           SDL_GetPerformanceCounter()); */


        /* if (seconds_elapsed < target_seconds_per_frame) { */
        /*     uint64_t time_to_sleep = */
        /*         (uint64_t)(((target_seconds_per_frame - seconds_elapsed) * */
        /*                     1000.0) - 1.0); */
        /*     if (time_to_sleep > 0) { */
        /*         SDL_Delay(time_to_sleep); */
        /*     } */

        /*     /\* wait the rest of the time till render *\/ */
        /*     while (sdl_get_seconds_elapsed(last_counter, */
        /*                                    SDL_GetPerformanceCounter()) < */
        /*            target_seconds_per_frame) { */
        /*         // waiting */
        /*     } */

        /* } */

        /* update the screen */
        SDL_GL_SwapWindow(window);

        uint64_t end_counter = SDL_GetPerformanceCounter();
        /* float full_frame_seconds_elapsed = */
        /*    sdl_get_seconds_elapsed(last_counter, end_counter); */
        /* printf("%f\n", full_frame_seconds_elapsed); */

        last_counter = end_counter;
    
    }
    
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
