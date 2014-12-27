#include <stdlib.h>
#include <stdbool.h>
#include <sdl2/sdl.h>


int main(int argc, char* argv[])
{
  int screen_width = 960;
  int screen_height = 540;
  bool running = true;

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

  SDL_Delay(1000);

  SDL_Quit();
}
