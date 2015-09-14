#include "rockets.h"
#include "stdarg.h"
// @NOTE: Draws the grid that the hexagon grid goes on top of.
// Used for debugging.

void
draw_base_grid(HexagonGrid grid)
{
    nvgSave(current_vg);
    nvgStrokeColor(current_vg, nvgRGBf(0.2, 0.2, 0.2));

    float width = grid.hexagon_size * 2;
    float height = sqrt(3)/2 * width;

    for (int column = 0; column <= grid.columns * 3 + 1; column++) {
        float line_base_x = grid.origin_x + (column * grid.hexagon_size/2);
        
        nvgBeginPath(current_vg);
        nvgMoveTo(current_vg, line_base_x, grid.origin_y);
        nvgLineTo(current_vg,
                  line_base_x,
                  grid.origin_y - grid.rows * height - height/2);
        nvgStroke(current_vg);
    }

    for (int row = 0; row <= grid.rows * 2 + 1; row++) {
        float line_base_y = grid.origin_y - (row * height/2);
        
        nvgBeginPath(current_vg);
        nvgMoveTo(current_vg, grid.origin_x, line_base_y);
        nvgLineTo(current_vg,
                  grid.origin_x + grid.columns * (width * 3/4) + (width * 1/4),
                  line_base_y);
        nvgStroke(current_vg);
    }

    nvgRestore(current_vg);
}

// @TODO: take a Color instead of an NVGcolor
void
draw_hex(HexagonGrid grid, GridV tile, NVGcolor color)
{
    float width = grid.hexagon_size * 2;
    float height = sqrt(3)/2 * width;

    V2 hex_center = gridV_to_pixel(grid, tile);

    nvgSave(current_vg);
    nvgStrokeColor(current_vg, color);
    
    nvgBeginPath(current_vg);
    nvgMoveTo(current_vg, hex_center.x - width/2, hex_center.y);
    nvgLineTo(current_vg, hex_center.x - width/4, hex_center.y + height/2);
    nvgLineTo(current_vg, hex_center.x + width/4, hex_center.y + height/2);
    nvgLineTo(current_vg, hex_center.x + width/2, hex_center.y);
    nvgLineTo(current_vg, hex_center.x + width/4, hex_center.y - height/2);
    nvgLineTo(current_vg, hex_center.x - width/4, hex_center.y - height/2);
    nvgLineTo(current_vg, hex_center.x - width/2, hex_center.y);
    nvgStroke(current_vg);

    nvgRestore(current_vg);
}

// @TODO: Have this use GridV, or just inline this in the grid drawing stuff.
void
draw_hex_tile(HexagonGrid grid, int x, int y)
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

    nvgBeginPath(current_vg);
    /* nvgCircle(vg, center_x, center_y, 3); */
    nvgMoveTo(current_vg, center_x - width/2, center_y);
    nvgLineTo(current_vg, center_x - width/4, center_y + height/2);
    nvgLineTo(current_vg, center_x + width/4, center_y + height/2);
    nvgLineTo(current_vg, center_x + width/2, center_y);
    nvgLineTo(current_vg, center_x + width/4, center_y - height/2);
    nvgLineTo(current_vg, center_x - width/4, center_y - height/2);
    nvgLineTo(current_vg, center_x - width/2, center_y);
    nvgStroke(current_vg);
}

// @OPTOMIZE: Stuff like this where you draw the whole vector grid each frame are not
// ideal.
void
draw_hex_grid(HexagonGrid grid)
{
    // @OPTOMIZE: This is super wastefull because it draws each line twice.
    nvgSave(current_vg);
    nvgStrokeColor(current_vg, nvgRGBf(0.75,0.75,0.75));
    // starts at 1 over height.
    for (int row = 0; row < grid.rows; row++) {
        for (int column = 0; column < grid.columns; column++) {
            draw_hex_tile(grid, column, row);
        }
    }
    nvgRestore(current_vg);
}

void
draw_ship(HexagonGrid grid, Position position, uint32_t thrusters)
{
    // Get screen loation.
    V2 pos = gridV_to_pixel(grid, position.tile);
    
    // Draw Rocket
    nvgSave(current_vg);
    nvgTranslate(current_vg, pos.x, pos.y);

    // @TODO: Rotation. Should be 60 degrees, counter clockwise per Direction.
    float PI_OVER_3 = -1.04719755;
    float rotation = position.facing * PI_OVER_3;

    nvgRotate(current_vg, rotation); // rotation in radians
    
    nvgFillColor(current_vg, nvgRGBf(1,0,0));
    nvgStrokeColor(current_vg, nvgRGBf(0,1,0));

    // body
    nvgBeginPath(current_vg);
    nvgRect(current_vg, -10.0, -25.0, 20.0, 40.0);
    nvgFill(current_vg);

    nvgBeginPath(current_vg);
    nvgRect(current_vg, -20.0, -5.0, 15.0, 30.0);
    nvgFill(current_vg);

    nvgBeginPath(current_vg);
    nvgRect(current_vg, 5.0, -5.0, 15.0, 30.0);
    nvgFill(current_vg);

    nvgBeginPath(current_vg);
    nvgCircle(current_vg, 0, 0, 3);
    nvgStroke(current_vg);
    
    nvgFillColor(current_vg, nvgRGBf(1, 1, 0));

    // thrusters
    if (thrusters_on(thrusters, BP)) {
        nvgBeginPath(current_vg);
        nvgRect(current_vg, -20.0, -25.0, 10, 10);
        nvgFill(current_vg);
    }

    if (thrusters_on(thrusters, BS)) {
        nvgBeginPath(current_vg);
        nvgRect(current_vg, 10.0, -25.0, 10, 10);
        nvgFill(current_vg);
    }

    if (thrusters_on(thrusters, SP)) {
        nvgBeginPath(current_vg);
        nvgRect(current_vg, -30.0, 15.0, 10, 10);
        nvgFill(current_vg);
    }

    if (thrusters_on(thrusters, SS)) {
        nvgBeginPath(current_vg);
        nvgRect(current_vg, 20.0, 15.0, 10, 10);
        nvgFill(current_vg);
    }

    if (thrusters_on(thrusters, BOOST)) {
        nvgBeginPath(current_vg);
        nvgRect(current_vg, -17.5, 25.0, 10, 10);
        nvgFill(current_vg);
        nvgBeginPath(current_vg);
        nvgRect(current_vg, 7.5, 25.0, 10, 10);
        nvgFill(current_vg);
    }

    nvgRestore(current_vg);
}

void draw_grid_arrow(V2 center, Direction pointing)
{
    nvgSave(current_vg);
    nvgTranslate(current_vg, center.x, center.y);

    float PI_OVER_3 = -1.0471975512;
    nvgRotate(current_vg, pointing * PI_OVER_3);

    nvgBeginPath(current_vg);
    nvgMoveTo(current_vg, 0, 20);
    nvgLineTo(current_vg, 0, -20);
    nvgLineTo(current_vg, 5, -15);
    nvgLineTo(current_vg, -5, -15);
    nvgLineTo(current_vg, 0, -20);
    nvgStrokeColor(current_vg, nvgRGBf(1,1,1));
    nvgStroke(current_vg);
    
    nvgRestore(current_vg);
}

// @TODO: Use Color
void draw_circle(V2 center, float r, Color color)
{
    nvgSave(current_vg);
    nvgBeginPath(current_vg);
    nvgCircle(current_vg, center.x, center.y, r);
    nvgStrokeColor(current_vg, nvgRGBf(0,1,0));
    nvgStroke(current_vg);
    nvgRestore(current_vg);
}

// @TODO: Use the color.
void
draw_formatted_text(V2 position, int size, Color color, const char* format, ...)
{
    char buffer[256] = {'\0'};
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, 256, format, args);

    nvgSave(current_vg);
    nvgFontSize(current_vg, size);
    nvgFillColor(current_vg, nvgRGBf(1,1,1));
    nvgText(current_vg, position.x, position.y, buffer, NULL);
    nvgRestore(current_vg);
}
