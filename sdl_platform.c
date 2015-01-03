#include <stdlib.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <SDL2/SDL.h>

#include "game.h"

// 2048‑by‑1536 is the real resolution
// todo(stephen): figure out if I need to draw to that for text.
#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 768

// todo(stephen): provide a way to statically link this for release builds.
#define GAME_LIBRARY "./libgame.so"
#define GAME_FUNCTION "game_update_and_render"

void *library_handle;
/* void (*game_update_and_render_fn)(uint32_t*, int); */
UpdateFunction game_update_and_render_fn;

float sdl_get_seconds_elapsed(uint64_t old_counter, uint64_t current_counter)
{
    return ((float)(current_counter - old_counter) /
            (float)SDL_GetPerformanceFrequency());
}

// I'm just prototyping on my mac for awhile. Not going to get set up for an actual ipad until later
// so for now just get going with the mouse.
bool sdl_handle_event(SDL_Event *event, int *other_events_this_tick, ControllerState *controller_state)
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
        printf("mouse down\n");
        controller_state->click = true;
        controller_state->is_dragging = true;
        break;

    case SDL_MOUSEBUTTONUP:
        printf("mouse up\n");
        controller_state->is_dragging = false;
        controller_state->end_dragging = true;
        break;
        // todo(stephen): there is a bug with window resizing here. I need the position to match the
        //                logical screen size, not the real one.
    case SDL_MOUSEMOTION:
        controller_state->mouse_x = event->motion.x;
        controller_state->mouse_y = event->motion.y;
        break;

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
    float game_update_hz = 30.0;
    float target_seconds_per_frame = 1.0 / game_update_hz;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        printf("Error initializing SDL: %s", SDL_GetError());
    }

    SDL_Window *window = SDL_CreateWindow("animal game",
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SDL_WINDOWPOS_UNDEFINED,
                                          SCREEN_WIDTH,
                                          SCREEN_HEIGHT,
                                          SDL_WINDOW_RESIZABLE);

    if (!window) {
      printf("Error creating window: %s", SDL_GetError());
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window,
                                                -1,
                                                0); // Is 0 accelerated?

    if (!renderer) {
      printf("Error creating renderer: %s", SDL_GetError());
    }

    /* Let sdl stretch our texture to the real window size. */
    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    SDL_Texture *texture = SDL_CreateTexture(renderer,
                                             SDL_PIXELFORMAT_ARGB8888,
                                             SDL_TEXTUREACCESS_STREAMING,
                                             SCREEN_WIDTH,
                                             SCREEN_HEIGHT);
    
    if (!texture) {
      printf("Error creating texture: %s", SDL_GetError());
    }


    PixelBuffer pixel_buffer = {.width = SCREEN_WIDTH,
                                .height = SCREEN_HEIGHT };
    void *gamestate;
    ControllerState controller_state = {.mouse_x = 0,
                                        .mouse_y = 0,
                                        .click = false,
                                        .is_dragging = false,
                                        .end_dragging = false};

    // Don't really know how big this is going to be yet.
    gamestate = malloc(10000);
    pixel_buffer.buffer =
        (uint32_t *) malloc(SCREEN_WIDTH * SCREEN_HEIGHT * 4);

    bool running = true;
    uint64_t last_counter = SDL_GetPerformanceCounter();

    while (running) {

        /* int width, height; */
        /* SDL_GetWindowSize(window, &width, &height); */
        /* printf("%dx%d\n", width, height); */

        /* reload game if it's changed */
        //todo(stephen): reload game only when it changes
        // look into kqueue
        if (library_handle != NULL) {
            dlclose(library_handle);
        }
      
        library_handle = dlopen(GAME_LIBRARY, RTLD_NOW);
        if (!library_handle) {
            printf("Error loading library %s\n", dlerror());
        }

        game_update_and_render_fn = dlsym(library_handle, GAME_FUNCTION);
        
        //todo(stephen): event handling
        SDL_Event event;
        int other_events_this_tick = 0;

        controller_state.end_dragging = false;;
        controller_state.click = false;
        
        while (SDL_PollEvent(&event)) {
            if (sdl_handle_event(&event, &other_events_this_tick, &controller_state)) {
                running = false;
            }
        }
        if (other_events_this_tick > 0) {
            printf("%d unhandled events this tick.\n", other_events_this_tick);
        }

        /* SDL_GetMouseState(&controller_state.mouse_x, &controller_state.mouse_y); */
    
        /* clear buffer */
        /* todo(stephen): decide if this is something you want
                        the game to handle instead. */
        for (int i = 0;
             i < SCREEN_WIDTH * SCREEN_HEIGHT;
            i++) {
            pixel_buffer.buffer[i] = 0;
        }

        /* run game tick */
        game_update_and_render_fn(pixel_buffer, gamestate, controller_state,
                                  target_seconds_per_frame);

        /* update texture */
        SDL_UpdateTexture(texture,
                          NULL,
                          pixel_buffer.buffer,
                          SCREEN_WIDTH * 4);
        SDL_RenderClear(renderer);
        SDL_RenderCopy(renderer, texture, NULL, NULL);

        /* delay the rest of the frame */
        int seconds_elapsed =
          sdl_get_seconds_elapsed(last_counter,
                                  SDL_GetPerformanceCounter());


        if (seconds_elapsed < target_seconds_per_frame) {
            uint64_t time_to_sleep =
                (uint64_t)(((target_seconds_per_frame - seconds_elapsed) *
                            1000.0) - 1.0);
            if (time_to_sleep > 0) {
                SDL_Delay(time_to_sleep);
            }

            /* wait the rest of the time till render */
            while (sdl_get_seconds_elapsed(last_counter,
                                           SDL_GetPerformanceCounter()) <
                   target_seconds_per_frame) {
                // waiting                
            }

        }

        /* update the screen */
        SDL_RenderPresent(renderer);

        uint64_t end_counter = SDL_GetPerformanceCounter();
        /* float full_frame_seconds_elapsed =
           sdl_get_seconds_elapsed(last_counter, end_counter); */
        last_counter = end_counter;
    
    }

    SDL_Quit();
}
