#include <stdlib.h>
#include <stdbool.h>
#include <dlfcn.h>
#include <SDL2/SDL.h>

#include "game.h"

// want game to really run at 1920 x 1080
// half that is 960 x 540
// but for comfortable development on my laptop I'm just going to develop at
#define SCREEN_WIDTH  869
#define SCREEN_HEIGHT 486

// todo(stephen): could move this stuff into the h file.
// can't these also be #define's
const char *GAME_LIBRARY = "./libgame.so";
const char *GAME_FUNCTION = "game_update_and_render";

void *library_handle;
/* void (*game_update_and_render_fn)(uint32_t*, int); */
UpdateFunction game_update_and_render_fn;

float sdl_get_seconds_elapsed(uint64_t old_counter, uint64_t current_counter)
{
    return ((float)(current_counter - old_counter) /
            (float)SDL_GetPerformanceFrequency());
}

bool sdl_handle_event(SDL_Event *event)
{
    bool should_quit = false;

    switch(event->type) {
    case SDL_QUIT:
        should_quit = true;
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
                                .height = SCREEN_HEIGHT};
    void *gamestate;

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
        while (SDL_PollEvent(&event)) {
            if (sdl_handle_event(&event)) {
                running = false;
            }
        }
    
        /* clear buffer */
        /* todo(stephen): decide if this is something you want
                        the game to handle instead. */
        for (int i = 0;
             i < SCREEN_WIDTH * SCREEN_HEIGHT;
            i++) {
            pixel_buffer.buffer[i] = 0;
        }

        /* run game tick */
        game_update_and_render_fn(pixel_buffer);

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
