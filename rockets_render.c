#include "rockets.h"
#include "stdarg.h"
// @NOTE: Draws the grid that the hexagon grid goes on top of.
// Used for debugging.

NVGcolor
get_color(Color color) {
    switch(color) {
    case(WHITE):
        return nvgRGBf(1,1,1);
        break;
    case(RED):
        return nvgRGBf(1,0,0);
        break;
    case(GREEN):
        return nvgRGBf(0,1,0);
        break;
    case(BLUE):
        return nvgRGBf(0,0,1);
    case(YELLOW):
        return nvgRGBf(1,1,0);
        break;
    case(CYAN):
        return nvgRGBf(0,1,1);
        break;
    case(MAGENTA):
        return nvgRGBf(1,0,1);
        break;
    case(BLACK):
        return nvgRGBf(0,0,0);
        break;
    }
}

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
draw_hex(HexagonGrid grid, GridV tile, Color color)
{
    float width = grid.hexagon_size * 2;
    float height = sqrt(3)/2 * width;

    V2 hex_center = gridV_to_pixel(grid, tile);

    nvgSave(current_vg);
    nvgStrokeColor(current_vg, get_color(color));
    
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

// @TODO: This is kind of a mess.
// @OPTOMIZE: Drawing this every frame is kind of expensive, maybe draw to a texture once.
void
draw_hex_grid(HexagonGrid grid)
{
    float width = grid.hexagon_size * 2;
    float height = sqrt(3)/2 * width;
    
    nvgSave(current_vg);
    nvgStrokeColor(current_vg, nvgRGBf(0.75,0.75,0.75));

    nvgTranslate(current_vg, grid.origin_x, grid.origin_y);

    // Draw Vertical Lines
    for (int column = 0; column <= grid.columns; column++) {
        float column_x = column * width*3/4;
    
        nvgBeginPath(current_vg);
        int column_segments = (grid.rows * 2);
        if (column != 0 &&
            (column != grid.columns || (grid.columns % 2 == 0))) {
            column_segments++;
        }
        
        bool left_leaning = (column % 2 == 0);

        if (left_leaning) {
            nvgMoveTo(current_vg, column_x + width/4, 0);
        } else {
            nvgMoveTo(current_vg, column_x, 0);
        }


        // @OPTOMIZE: This is pretty awful for performance, rethink all this drawing.
        bool first = true;
        for (int segment = 1; segment <= column_segments; segment++) {
            if (left_leaning) {
                if (first && column == grid.columns && column % 2 == 0) {
                    nvgMoveTo(current_vg, column_x, segment * -height/2);
                    first = false;
                } else {
                    nvgLineTo(current_vg, column_x, segment * -height/2);
                }
            } else {
                nvgLineTo(current_vg, column_x + width/4, segment * -height/2);
            }
            left_leaning = !left_leaning;
        }
        nvgStroke(current_vg);
    }

    // Draw Horisontal Lines
    float column_x = width/4;
    for (int column = 0; column < grid.columns; column++) {        
        for (int row = 0; row <= grid.rows; row++) {
            float row_y = -height * row;
            if (column % 2 == 1) {
                row_y -= height/2;
            }
            
            nvgBeginPath(current_vg);
            nvgMoveTo(current_vg, column_x, row_y);
            nvgLineTo(current_vg, column_x + width/2, row_y);
            nvgStroke(current_vg);
        }
        column_x += width * 3/4;
    }
    
    nvgRestore(current_vg);
}

void
draw_ship(HexagonGrid grid, Position position, uint32_t thrusters,
          Color body_color, Color thruster_color)
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
    
    nvgFillColor(current_vg, get_color(body_color));
    /* nvgStrokeColor(current_vg, nvgRGBf(0,1,0)); */

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
    
    nvgFillColor(current_vg, get_color(thruster_color));

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

void draw_grid_arrow(V2 center, Direction pointing, Color color)
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
    nvgStrokeColor(current_vg, get_color(color));
    nvgStroke(current_vg);
    
    nvgRestore(current_vg);
}

// @TODO: Use Color
void draw_circle(V2 center, float r, Color color)
{
    nvgSave(current_vg);
    nvgBeginPath(current_vg);
    nvgCircle(current_vg, center.x, center.y, r);
    nvgStrokeColor(current_vg, get_color(color));
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
    nvgFillColor(current_vg, get_color(color));
    nvgText(current_vg, position.x, position.y, buffer, NULL);
    nvgRestore(current_vg);
}
