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
    state->ship_position.facing = UP;

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

    if (tickframe) {
        log_info("Last Position: (%i,%i,%i)", state->ship_position.tile.x, state->ship_position.tile.y, state->ship_position.tile.z);
        
        
        log_info("Next Position: (%i,%i,%i)", next_position.tile.x, next_position.tile.y, next_position.tile.z);
    }
    
    V2 center = gridV_to_pixel(grid, next_position.tile);

    nvgSave(vg);
    nvgTranslate(vg, center.x, center.y);
    float PI_OVER_3 = 1.0471975512;
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
    
#if 0
    /* int x, y; */
    /* if (grid_get_tile_for_screen_coordinates(&x, &y, grid, input.mouse_x, input.mouse_y)) { */
    /*     char buf[32] = {'\0'}; */
    /*     snprintf(buf, 32, "x: %i, y: %i", x, y); */

    /*     nvgSave(vg); */
    /*     nvgFontSize(vg, 14); */
    /*     nvgFillColor(vg, nvgRGBf(1, 1, 1)); */
    /*     nvgText(vg, 10, 10, buf, NULL); */
    /*     nvgRestore(vg); */
        
    /*     // Show which grid the mouse is over. */
    /*     nvgSave(vg); */
    /*     nvgStrokeColor(vg, nvgRGBf(1, 0, 0)); */
    /*     /\* draw_hex_tile(vg, grid, x, y); *\/ */
    /*     nvgRestore(vg); */
    /* } */

    /* float i, j; */
    /* grid_get_coordinates_for_tile(&i, &j, grid, 1, 0); */
    /* nvgSave(vg); */
    /* nvgBeginPath(vg); */
    /* nvgCircle(vg, i, j, 3); */
    /* nvgStrokeColor(vg, nvgRGBf(1,1,1)); */
    /* nvgStroke(vg); */
    /* nvgRestore(vg); */

    // Show the rocket.
    /* float width = grid.hexagon_size * 2; */
    /* float height = sqrt(3)/2 * width; */

    if (state->tick++ % 60 == 0) {
        state->ship_thrusters++;

        if (state->ship_thrusters == 32) {
            state->ship_thrusters = 0;
        }
    }

    char t[32] = {'\0'};
    snprintf(t, 32, "tick: %i", state->tick);

    nvgSave(vg);
    nvgFontSize(vg, 14);
    nvgFillColor(vg, nvgRGBf(1, 1, 1));
    nvgText(vg, 10, 10, t, NULL);
    nvgRestore(vg);
    Position ship_position = state->ship_position;

    float i, j;
    grid_get_coordinates_for_tile(&i, &j, grid, ship_position.x, ship_position.y);

    // ROCKET RENDER WAS HERE

    Position next_position = next_ship_position(ship_position, thrusters);
    grid_get_coordinates_for_tile(&i, &j, grid, next_position.x, next_position.y);
    nvgSave(vg);
    nvgTranslate(vg, i, j);
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

    /* for (int t = 0; t < 32; t++) { */
    /*     Position next_position = next_ship_position(ship_position, t); */
    /*     grid_get_coordinates_for_tile(&i, &j, grid, next_position.x, next_position.y); */

    /*     nvgSave(vg); */
    /*     nvgTranslate(vg, i, j); */
    /*     nvgRotate(vg, next_position.facing * PI_OVER_3); */

    /*     nvgBeginPath(vg); */
    /*     nvgMoveTo(vg, 0, 20); */
    /*     nvgLineTo(vg, 0, -20); */
    /*     nvgLineTo(vg, 5, -15); */
    /*     nvgLineTo(vg, -5, -15); */
    /*     nvgLineTo(vg, 0, -20); */
    /*     nvgStrokeColor(vg, nvgRGBf(1,1,1)); */
    /*     nvgStroke(vg); */
    /*     nvgRestore(vg); */
    /* } */
#endif
}

const gg_Game gg_game_api = {
    .gamestate_size = sizeof(GameState),
    .init = game_setup,
    .update_and_render = game_update_and_render
};

/* DebugRecord debug_records[__COUNTER__]; */
/* int num_debug_records = ARRAY_COUNT(debug_records); */
