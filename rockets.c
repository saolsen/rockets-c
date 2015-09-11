#include "rockets.h"

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

    state->ship_position.x = 13;
    state->ship_position.y = 3;
    state->ship_position.facing = UP;

    state->tick = 0;

    return state;
}

// Hexagon Math!
#define hexagon_grid_origin_x 20
#define hexagon_grid_origin_y 675
#define hexagon_side_length 20

// @NOTE: Draws the grid that the hexagon grid goes on top of.
// Used for debugging.
// @TODO: I messed up the math.
// height = sqrt(3)/2 * width. not 2*size
void
draw_base_grid(NVGcontext* vg, HexagonGrid grid)
{
    nvgSave(vg);
    nvgStrokeColor(vg, nvgRGBf(0.2, 0.2, 0.2));

    float width = grid.hexagon_size * 2;
    float height = sqrt(3)/2 * width;

    for (int column = 0; column <= grid.columns * 3 + 1; column++) {
        float line_base_x = grid.origin_x + (column * grid.hexagon_size/2);
        
        nvgBeginPath(vg);
        nvgMoveTo(vg, line_base_x, grid.origin_y);
        nvgLineTo(vg,
                  line_base_x,
                  grid.origin_y - grid.rows * height - height/2);
        nvgStroke(vg);
    }

    for (int row = 0; row <= grid.rows * 2 + 1; row++) {
        float line_base_y = grid.origin_y - (row * height/2);
        
        nvgBeginPath(vg);
        nvgMoveTo(vg, grid.origin_x, line_base_y);
        nvgLineTo(vg,
                  grid.origin_x + grid.columns * (width * 3/4) + (width * 1/4),
                  line_base_y);
        nvgStroke(vg);
    }

    nvgRestore(vg);
}

void
draw_hex_tile(NVGcontext* vg, HexagonGrid grid, int x, int y)
{
    assert(x >= 0);
    assert(y >= 0);
    assert(x < grid.columns);
    assert(y < grid.rows);

    float width = grid.hexagon_size * 2;
    float height = sqrt(3)/2 * width;

    float center_x = (grid.origin_x + grid.hexagon_size) + (grid.hexagon_size * 1.5 * x);
    float center_y = (grid.origin_y - height/2) - (height * y);
    
    if (x % 2 == 1) {
        center_y -= height/2;
    }

    nvgBeginPath(vg);
    /* nvgCircle(vg, center_x, center_y, 3); */
    nvgMoveTo(vg, center_x - width/2, center_y);
    nvgLineTo(vg, center_x - width/4, center_y + height/2);
    nvgLineTo(vg, center_x + width/4, center_y + height/2);
    nvgLineTo(vg, center_x + width/2, center_y);
    nvgLineTo(vg, center_x + width/4, center_y - height/2);
    nvgLineTo(vg, center_x - width/4, center_y - height/2);
    nvgLineTo(vg, center_x - width/2, center_y);
    nvgStroke(vg);
}

// @OPTOMIZE: Stuff like this where you draw the whole vector grid each frame are not
// ideal.
void
draw_hex_grid(NVGcontext* vg, HexagonGrid grid)
{
    // @OPTOMIZE: This is super wastefull because it draws each line twice.
    nvgSave(vg);
    nvgStrokeColor(vg, nvgRGBf(0.75,0.75,0.75));
    // starts at 1 over height.
    for (int row = 0; row < grid.rows; row++) {
        for (int column = 0; column < grid.columns; column++) {
            draw_hex_tile(vg, grid, column, row);
        }
    }
    nvgRestore(vg);
}

// returns false if mouse is not over screen.
bool
grid_get_tile_for_screen_coordinates(int* x, int* y, HexagonGrid grid, float mouse_x, float mouse_y)
{
    float tile_x = grid.origin_x - mouse_x;
    float tile_y = -(grid.origin_y - mouse_y);
    
    // math....
    *x = tile_x;
    *y = tile_y;

    return true;
}

// Sets x and y to the center of the tile.
bool
grid_get_coordinates_for_tile(float* x, float* y, HexagonGrid grid, int tile_x, int tile_y)
{
    if (tile_x < 0 || tile_x > grid.columns ||
        tile_y < 0 || tile_y > grid.rows) {
        return false;
    }
   
    float width = grid.hexagon_size * 2;
    float height = sqrt(3)/2 * width;

    float center_x = (grid.origin_x + grid.hexagon_size) + (grid.hexagon_size * 1.5 * tile_x);
    float center_y = (grid.origin_y - height/2) - (height * tile_y);

    if (tile_x % 2 == 1) {
        center_y -= height/2;
    }

    *x = center_x;
    *y = center_y;

    return true;    
}

bool
thrusters_set(uint32_t ship_thrusters, uint32_t check_thrusters)
{
    return ship_thrusters & check_thrusters;
}

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

    /* draw_base_grid(vg, grid); */
    draw_hex_grid(vg, grid);

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

    nvgSave(vg);

    nvgTranslate(vg, i, j);

    // @TODO: Rotation. Should be 60 degrees, counter clockwise per Direction.
    float PI_OVER_3 = -1.04719755;
    float rotation = ship_position.facing * PI_OVER_3;

    nvgRotate(vg, rotation); // rotation in radians
    
    nvgFillColor(vg, nvgRGBf(1,0,0));
    nvgStrokeColor(vg, nvgRGBf(0,1,0));

    // body
    nvgBeginPath(vg);
    nvgRect(vg, -10.0, -25.0, 20.0, 40.0);
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRect(vg, -20.0, -5.0, 15.0, 30.0);
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgRect(vg, 5.0, -5.0, 15.0, 30.0);
    nvgFill(vg);

    nvgBeginPath(vg);
    nvgCircle(vg, 0, 0, 3);
    nvgStroke(vg);
    
    uint32_t thrusters = state->ship_thrusters;

    /* thrusters = 0;   // 1, 2, 4, 7, 8, 11, 13, 14, */

    nvgFillColor(vg, nvgRGBf(1, 1, 0));

    // thrusters
    if (thrusters_set(thrusters, BP)) {
        nvgBeginPath(vg);
        nvgRect(vg, -20.0, -25.0, 10, 10);
        nvgFill(vg);
    }

    if (thrusters_set(thrusters, BS)) {
        nvgBeginPath(vg);
        nvgRect(vg, 10.0, -25.0, 10, 10);
        nvgFill(vg);
    }

    if (thrusters_set(thrusters, SP)) {
        nvgBeginPath(vg);
        nvgRect(vg, -30.0, 15.0, 10, 10);
        nvgFill(vg);
    }

    if (thrusters_set(thrusters, SS)) {
        nvgBeginPath(vg);
        nvgRect(vg, 20.0, 15.0, 10, 10);
        nvgFill(vg);
    }

    if (thrusters_set(thrusters, BOOST)) {
        nvgBeginPath(vg);
        nvgRect(vg, -17.5, 25.0, 10, 10);
        nvgFill(vg);
        nvgBeginPath(vg);
        nvgRect(vg, 7.5, 25.0, 10, 10);
        nvgFill(vg);
    }

    nvgRestore(vg);

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
}

const gg_Game gg_game_api = {
    .gamestate_size = sizeof(GameState),
    .init = game_setup,
    .update_and_render = game_update_and_render
};

/* DebugRecord debug_records[__COUNTER__]; */
/* int num_debug_records = ARRAY_COUNT(debug_records); */
