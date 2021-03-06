#include <SDL2/SDL.h>
#include <OpenGL/gl3.h>

#include <stdlib.h>
#include <stdbool.h>
// @TODO: Do reloading in a platform independent way.
#include <dlfcn.h>
// @TODO: Do random numbers in a platform independent way.
#include <time.h>

#include "rockets_platform.h"
#define NANOVG_GL3_IMPLEMENTATION
#include "nanovg_gl.h"

// Library functions for game.
uint64_t performance_counter()
{
    return SDL_GetPerformanceCounter();
}

uint64_t performance_frequency()
{
    return SDL_GetPerformanceFrequency();
}

// @TODO: Include in game config object.
// @TODO: Screen resizing does not work.

typedef struct gg_CurrentGame {
    void *handle;
    ino_t id;
    gg_Game game;
} gg_CurrentGame;

void
game_reload(gg_CurrentGame* current_game, char* library)
{
    struct stat attr;
    if ((stat(library, &attr) == 0) && (current_game->id != attr.st_ino)) {
        log_info("New Library to load.");
        
        if (current_game->handle) {
            dlclose(current_game->handle);
        }
        
        void* handle = dlopen(library, RTLD_NOW);

        if (handle) {
            current_game->handle = handle;
            current_game->id = attr.st_ino;

            gg_Game* game = dlsym(handle, "gg_game_api");
            if (game != NULL) {
                current_game->game = *game;
                log_info("Reloaded Game Library");
                
            } else {
                log_error("Error loading api symbol.");
                dlclose(handle);
                current_game->handle = NULL;
                current_game->id = 0;
            }
            
        } else {
            log_error("Error loading game library.");
            current_game->handle = NULL;
            current_game->id = 0;
        }   
    }
}


float
gg_get_seconds_elapsed(uint64_t old_counter, uint64_t current_counter)
{
    return ((float)(current_counter - old_counter) /
            (float)SDL_GetPerformanceFrequency());
}

// @TODO: Have a way to record input over time, possibly using a
//                persistent data structure if that turns out to be helpful.
bool
gg_handle_event(SDL_Event *event, int *other_events_this_tick,
        gg_Input *input_state)
{
    bool should_quit = false;

    switch(event->type) {
        case SDL_QUIT:
            should_quit = true;
            break;

            // @NOTE: Going to use mouse events for now.
            // @TODO: use touch events when ready to port to ipad.
            // @TODO: figure out how to just do a click.
            // @NOTE: might want zooming
        case SDL_MOUSEBUTTONDOWN:
            /* log_info("mouse down"); */
            input_state->down_time = SDL_GetTicks();
            /* input_state->click = true; */
            input_state->start_dragging = true;
            input_state->is_dragging = true;

            input_state->mouse_down = true;
            break;

        case SDL_MOUSEBUTTONUP:
            if (!input_state->holding) {
                input_state->click = true;
            }
            input_state->down_time = 0;
            input_state->holding = false;
            input_state->end_dragging = true;
            input_state->is_dragging = false;

            input_state->mouse_up = true;
            break;

        case SDL_MOUSEMOTION:
            input_state->mouse_x = event->motion.x;
            input_state->mouse_y = event->motion.y;

            // @TODO: This relative mouse position is mad wrong.
            // I should probably just keep track of this myself.
            input_state->mouse_motion = true;
            input_state->mouse_xrel = event->motion.xrel;
            input_state->mouse_yrel = event->motion.yrel;
            break;

            // @TODO: SDL_SetWindowFullscreen when hitting f.
        case SDL_KEYDOWN: {
            log_info("keypress");
            /* log_info("keypress"); */
            SDL_Keycode keycode = event->key.keysym.sym;

            if (SDLK_ESCAPE == keycode) {
                should_quit = true;
            }
        } break;

        default:
            // @TODO: print out that other events happened for debugging
            *other_events_this_tick+= 1;
            break;
    }

    return should_quit;
}


int main(int argc, char* argv[])
{

    gg_CurrentGame current_game;
    char* game_library = "libgame.dylib";
    game_reload(&current_game, game_library);

    // @TODO: Figure out a better way to do random numbers.
    srand(time(NULL));

    // @TODO: Make this part of the config object.
    float game_update_hz = 60.0;
    float target_seconds_per_frame = 1.0f / game_update_hz;

    // @TODO: have a better error handling scheme that has all of these
    //        goto the end and write an error message instead of a bunch
    //        of different checks.
    if (0 != SDL_Init(SDL_INIT_VIDEO)) {
        log_info("Error initializing SDL: %s", SDL_GetError());
    }

    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    // @TODO: Have window name come from config object.
    SDL_Window *window = SDL_CreateWindow("rockets",
                                          SDL_WINDOWPOS_CENTERED,
                                          SDL_WINDOWPOS_CENTERED,
                                          SCREEN_WIDTH,
                                          SCREEN_HEIGHT,
                                          SDL_WINDOW_OPENGL |
                                          SDL_WINDOW_RESIZABLE |
                                          SDL_WINDOW_ALLOW_HIGHDPI);

    int w, h;
    SDL_GL_GetDrawableSize(window, &w, &h);
    log_info("Drawable Resolution: %d x %d", w, h);

    if (!window) {
        log_info("Error creating window: %s", SDL_GetError());
    }

    SDL_GLContext context = SDL_GL_CreateContext(window);

    // Use Vsync
    if (SDL_GL_SetSwapInterval(1) < 0) {
        log_info( "Warning: Unable to set VSync! SDL Error: %s", SDL_GetError());
    }

    // Setup nvg
    NVGcontext* vg = nvgCreateGL3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
    if (vg == NULL) {
        log_error("Could not init nanovg.");
    }

    //@TODO: IF DEBUG
    /* gg_Debug_vg = vg; */

    // Init Game
    void *gamestate = calloc(1, current_game.game.gamestate_size);
    gamestate = current_game.game.init(gamestate, vg);

    gg_Input input = {};

    bool running = true;
    int update_time = 0;
    int frame_time = 0;

    uint64_t last_counter = SDL_GetPerformanceCounter();
    int last_start_time = SDL_GetTicks();
    

    while (running) {
        int start_time = SDL_GetTicks();
        frame_time = start_time - last_start_time;
        last_start_time = start_time;
        
        // Reload Library
        game_reload(&current_game, game_library);

        //@TODO: better event handling
        SDL_Event event;
        int other_events_this_tick = 0;

        input.start_dragging = false;
        input.end_dragging = false;
        input.click = false;
        input.mouse_motion = false;

        input.mouse_down = false;
        input.mouse_up = false;
        
        while (SDL_PollEvent(&event)) {
            if (gg_handle_event(&event,
                                &other_events_this_tick,
                                &input)) {
                running = false;
            }
        }

        // Decide if the mouse is being held down?
        if (input.down_time != 0) {
            if (SDL_GetTicks() - input.down_time > 500) {
                input.holding = true;
            }
        }

        if (other_events_this_tick > 0) {
            /* log_info("%d unhandled events this tick.",
               other_events_this_tick); */
        }

        /* Clear Screen */
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT |
                GL_STENCIL_BUFFER_BIT);

        // @TODO: Support non retina.
        nvgBeginFrame(vg, SCREEN_WIDTH, SCREEN_HEIGHT, 2.0);

        /* run game tick */
        current_game.game.update_and_render(gamestate, vg, input,
                                            target_seconds_per_frame);

        /* end frame */
        update_time = SDL_GetTicks() - start_time;

        char stats[128];
        snprintf(stats, 128,
                 "game: %ims, frame: %ims, fps: %.0f",
                 update_time, frame_time, 1.0/(frame_time * 0.001));
        nvgSave(vg);
        nvgFontSize(vg, 24);
        nvgFillColor(vg, nvgRGBf(1,1,1));
        nvgText(vg, 5, (h/2)-10, stats, NULL);
        nvgRestore(vg);

        nvgEndFrame(vg);

        // Try to delay and see if that helps battery.
        // @TODO: Somehow figure this out for real.
        int time_till_vsync = target_seconds_per_frame*1000.0 - (SDL_GetTicks() - start_time);
        if (time_till_vsync > 3) {
            SDL_Delay(time_till_vsync - 2);
        }
        
        SDL_GL_SwapWindow(window);

        uint64_t end_counter = SDL_GetPerformanceCounter();
        last_counter = end_counter;

    }

    // Clean Up
    log_info("Cleaning up");
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}
