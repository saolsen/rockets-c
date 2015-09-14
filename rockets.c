#include "rockets.h"

#include "rockets_grid.c"
#include "rockets_sim.c"
#include "rockets_gui.c"
#include "rockets_render.c"

static void*
game_setup(void* game_state, NVGcontext* vg)
{
    log_info("Setting up game");
    GameState* state = (GameState*)game_state;

    // @TODO: If you have more than one font you need to store a
    // reference to this.
    int font = nvgCreateFont(vg,
                             "basic",
                             "SourceSansPro-Regular.ttf");
    // Assert that the font got loaded.
    assert(font >= 0);

    state->tick = 0;

    state->ship_position.tile = gridV(13, -1, -12);
    state->ship_position.facing = RIGHT_UP;

    current_gui_state = &(state->gui_state);

    return state;
}

// Hexagon Math!
#define hexagon_grid_origin_x 20
#define hexagon_grid_origin_y 675

static void
game_update_and_render(void* gamestate,
                       NVGcontext* vg,
                       gg_Input input,
                       float dt)
{
    GameState* state = (GameState*)gamestate;

    current_vg = vg;

    // @HARDCODE: screen size
    gui_init(input, 1280, 720, dt);
    
    HexagonGrid grid = {.rows = 12,
                        .columns = 27,
                        .origin_x = hexagon_grid_origin_x,
                        .origin_y = hexagon_grid_origin_y,
                        .hexagon_size = 30};

    if (state->tick++ % 60 == 0) {
        state->ship_thrusters++;

        if (state->ship_thrusters == 32) {
            state->ship_thrusters = 0;
        }
    }

    draw_base_grid(grid);
    draw_hex_grid(grid);
    draw_ship(grid, state->ship_position, state->ship_thrusters, RED, YELLOW);

    Position next_position = next_ship_position(state->ship_position, state->ship_thrusters);
    V2 center = gridV_to_pixel(grid, next_position.tile);
    draw_grid_arrow(center, next_position.facing, WHITE);

    GridV mouse_over = pixel_to_gridV(grid, v2(input.mouse_x, input.mouse_y));
    draw_formatted_text(v2(5,15), 24, WHITE,
                        "(%i,%i,%i)",
                        mouse_over.x, mouse_over.y, mouse_over.z);

    draw_hex(grid, mouse_over, CYAN);

    // show mouse
    draw_circle(v2(input.mouse_x, input.mouse_y), 3, GREEN);

    gui_render(vg);
}

const gg_Game gg_game_api = {
    .gamestate_size = sizeof(GameState),
    .init = game_setup,
    .update_and_render = game_update_and_render
};

/* DebugRecord debug_records[__COUNTER__]; */
/* int num_debug_records = ARRAY_COUNT(debug_records); */
