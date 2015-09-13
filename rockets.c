#include "rockets.h"

#include "rockets_grid.c"
#include "rockets_sim.c"
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
    
    HexagonGrid grid = {.rows = 12,
                        .columns = 27,
                        .origin_x = hexagon_grid_origin_x,
                        .origin_y = hexagon_grid_origin_y,
                        .hexagon_size = 30};

    bool tickframe = false;
    if (state->tick++ % 60 == 0) {
        state->ship_thrusters++;
        tickframe = true;

        if (state->ship_thrusters == 32) {
            state->ship_thrusters = 0;
        }
    }

    draw_base_grid(vg, grid);
    draw_hex_grid(vg, grid);
    draw_ship(vg, grid, state->ship_position, state->ship_thrusters);

    Position next_position = next_ship_position(state->ship_position, state->ship_thrusters);
    V2 center = gridV_to_pixel(grid, next_position.tile);

    if (tickframe) {
        /* log_info("Last Position: (%i,%i,%i), %i", */
        /*          state->ship_position.tile.x, */
        /*          state->ship_position.tile.y, */
        /*          state->ship_position.tile.z, */
        /*          state->ship_position.facing); */
        /* log_info("Next Position: (%i,%i,%i), %i", */
        /*          next_position.tile.x, */
        /*          next_position.tile.y, */
        /*          next_position.tile.z, */
        /*          next_position.facing); */

        /* log_info("Center: (%f, %f)", center.x, center.y); */
    }

    nvgSave(vg);
    nvgTranslate(vg, center.x, center.y);

    float PI_OVER_3 = -1.0471975512;
    nvgRotate(vg, next_position.facing * PI_OVER_3);

    nvgBeginPath(vg);
    nvgMoveTo(vg, 0, 20);
    nvgLineTo(vg, 0, -20);
    nvgLineTo(vg, 5, -15);
    nvgLineTo(vg, -5, -15);
    nvgLineTo(vg, 0, -20);
    nvgStrokeColor(vg, nvgRGBf(1,1,1));
    nvgStroke(vg);
    
    nvgRestore(vg);

    GridV mouse_over = pixel_to_gridV(grid, v2(input.mouse_x, input.mouse_y));


    char spot[128];
    snprintf(spot, 128,
             "(%i,%i,%i)",
             mouse_over.x, mouse_over.y, mouse_over.z);
    nvgSave(vg);
    nvgFontSize(vg, 24);
    nvgFillColor(vg, nvgRGBf(1,1,1));
    nvgText(vg, 5, 15, spot, NULL);
    nvgRestore(vg);

    draw_hex(vg, grid, mouse_over, nvgRGBf(1,0,0));
    
    
}

const gg_Game gg_game_api = {
    .gamestate_size = sizeof(GameState),
    .init = game_setup,
    .update_and_render = game_update_and_render
};

/* DebugRecord debug_records[__COUNTER__]; */
/* int num_debug_records = ARRAY_COUNT(debug_records); */
