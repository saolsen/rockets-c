#include <stdlib.h>
#include <stdbool.h>
#include <sdl2/sdl.h>


int main(int argc, char* argv[])
{
  int screen_width = 960;
  int screen_height = 540;

  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    printf("Error initializing SDL: %s", SDL_GetError());
  }

  SDL_Window *window = SDL_CreateWindow("animal game",
                                        SDL_WINDOWPOS_UNDEFINED,
                                        SDL_WINDOWPOS_UNDEFINED,
                                        screen_width,
                                        screen_height,
                                        SDL_WINDOW_RESIZABLE);

  if (!window) printf("Error creating window: %s", SDL_GetError());

  SDL_Renderer *renderer = SDL_CreateRenderer(window,
                                              -1,
                                              0); // Is 0 accelerated?

  if (!renderer) printf("Error creating renderer: %s", SDL_GetError());

  /* Let sdl stretch our texture to the real window size. */
  SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
  SDL_RenderSetLogicalSize(renderer, screen_width, screen_height);

  SDL_Texture *texture = SDL_CreateTexture(renderer,
                                           SDL_PIXELFORMAT_ARGB8888,
                                           SDL_TEXTUREACCESS_STREAMING,
                                           screen_width,
                                           screen_height);

  if (!texture) printf("Error creating texture: %s", SDL_GetError());

  uint32_t *pixel_buffer;
  void *gamestate;

  // Don't really know how big this is going to be yet.
  gamestate = malloc(10000);
  pixel_buffer = (uint32_t *) malloc(screen_width * screen_height * 4);

  bool running = true;

  while (running) {

    //todo(stephen): event handling
    
    //todo(stephen): game update and render

    //todo(stephen): delay the rest of the frame

    //todo(stephen): update the screen
    
    SDL_Delay(1000);
    break;
    
  }

  SDL_Quit();
}
