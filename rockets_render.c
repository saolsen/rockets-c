#include "rockets.h"
/*
  I'm a little stuck on translating between screen and grid coordinates.
  Need to figure out a few things.

  * Based on where I start indexing how do I know where the bounds of the grid are?
  * How do I go from screen coordinates to grid coordinates.
  * How do I go from grid coordinates to screen coordinates.

  I think that maybe there is a function between a GridV and a screen x,y.
  That should be easy to find for going from GridV to screen, but it will be
  a lot harder to find for going from screen to GridV because only the center will
  translate properly. Maybe first I can just try to go from GridV to screen.

 */

// @NOTE: Draws the grid that the hexagon grid goes on top of.
// Used for debugging.
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

void
draw_ship(NVGcontext* vg, HexagonGrid grid, GridV position, Direction facing, uint32_t thrusters)
{
    // Get screen loation.
    V2 pos = gridV_to_pixel(grid, position);
    
    // Draw Rocket
    nvgSave(vg);
    nvgTranslate(vg, pos.x, pos.y);

    // @TODO: Rotation. Should be 60 degrees, counter clockwise per Direction.
    float PI_OVER_3 = -1.04719755;
    float rotation = facing * PI_OVER_3;

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
    
    nvgFillColor(vg, nvgRGBf(1, 1, 0));

    // thrusters
    if (thrusters_on(thrusters, BP)) {
        nvgBeginPath(vg);
        nvgRect(vg, -20.0, -25.0, 10, 10);
        nvgFill(vg);
    }

    if (thrusters_on(thrusters, BS)) {
        nvgBeginPath(vg);
        nvgRect(vg, 10.0, -25.0, 10, 10);
        nvgFill(vg);
    }

    if (thrusters_on(thrusters, SP)) {
        nvgBeginPath(vg);
        nvgRect(vg, -30.0, 15.0, 10, 10);
        nvgFill(vg);
    }

    if (thrusters_on(thrusters, SS)) {
        nvgBeginPath(vg);
        nvgRect(vg, 20.0, 15.0, 10, 10);
        nvgFill(vg);
    }

    if (thrusters_on(thrusters, BOOST)) {
        nvgBeginPath(vg);
        nvgRect(vg, -17.5, 25.0, 10, 10);
        nvgFill(vg);
        nvgBeginPath(vg);
        nvgRect(vg, 7.5, 25.0, 10, 10);
        nvgFill(vg);
    }

    nvgRestore(vg);
}
