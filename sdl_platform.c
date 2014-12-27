#include <stdlib.h>
#include <stdbool.h>
#include <sdl2/sdl.h>

// STUFF THAT SHOULD GO IN THE GAME AND NOT HERE

int round_to_int(float n) {
  if (n >= 0.0) {
    return (int)(n + 0.5);
  } else {
    return (int)(n - 0.5);
  }
}

uint32_t round_to_uint(float n) {
  if (n >= 0.0) {
    return (uint32_t)(n + 0.5);
  } else {
    return (uint32_t)(n - 0.5);
  }
}

void draw_rectangle(uint32_t pixel_buffer[],
                    uint32_t pitch, //aka, screen_width
                    float top_left_x,
                    float top_left_y,
                    float width,
                    float height,
                    float red, float green, float blue)
{
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

      int pixel = (row * pitch) + column;

      pixel_buffer[pixel] = value;
    }
  }
}
                    

// END

bool sdl_handle_event(SDL_Event *event) {
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
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      if (sdl_handle_event(&event)) {
        running = false;
      }
    }
    
    //todo(stephen): game update and render
    // testing
    draw_rectangle(pixel_buffer, screen_width, 100.0, 100.0, 25.0, 25.0, 1.0, 1.0, 0.0);

    //todo(stephen): live reload of game

    //todo(stephen): delay the rest of the frame

    //todo(stephen): update the screen
    SDL_UpdateTexture(texture, NULL, pixel_buffer, screen_width * 4);
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, texture, NULL, NULL);
    SDL_RenderPresent(renderer);
    
    /* SDL_Delay(1000); */
    /* break; */
    
  }

  SDL_Quit();
}
