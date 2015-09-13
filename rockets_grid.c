// @TODO: drop this include because it's a unity build.
#include "rockets.h"

Direction
rotate_direction(Direction direction, int rotation)
{
    int dir = (int)direction;
    dir += rotation;
    dir = (dir + 6) % 6;
    return (Direction)dir;
}


GridV
gridV(int x, int y, int z)
{
    return (GridV){.x = x, .y = y, .z = z};
}


int
gridV_eq(GridV a, GridV b)
{
    return (a.x == b.x &&
            a.y == b.y &&
            a.z == b.z);
}


GridV
gridV_plus(GridV a, GridV b)
{
    a.x += b.x;
    a.y += b.y;
    a.z += b.z;
    return a;
}


int
gridV_distance(GridV a, GridV b)
{
    return (abs(a.x - b.x) + abs(a.y - b.y) + abs(a.z - b.z)) / 2;
}


int
gridV_magnitude(GridV v)
{
    return gridV_distance(GRID_ZERO, v);
}


// @NOTE: This takes a float so you can do stuff like 0.5;
// You'll still get back ints so don't mess yourself up.
// @TODO: Could make this gridV_divide and take an int instead.
GridV
gridV_scale(GridV v, float n)
{
    v.x = (int)(v.x * n);
    v.y = (int)(v.y * n);
    v.z = (int)(v.z * n);
    return v;
}

// Rotate a grid vector counter clockwise around the hexagon.
GridV
gridV_rotate(GridV v, int rotation)
{
    // Only rotating left for now.
    int left_rotation = (rotation + 6) % 6;
    
    // A left rotation is
    //[ x,y,z] to [-y, -z, -x]
    for (int i = 0; i < left_rotation; i++) {
        GridV new_v = gridV(-v.y, -v.z, -v.x);
        v = new_v;
    }

    return v;
}

// Get the unit (1 space) grid vector for a given direction.
GridV
gridV_for_direction(Direction direction)
{
    switch(direction) {
    case(UP):
        return GRID_UP;
        break;
    case(LEFT_UP):
        return GRID_LEFT_UP;
        break;
    case(LEFT_DOWN):
        return GRID_LEFT_DOWN;
        break;
    case(DOWN):
        return GRID_DOWN;
        break;
    case(RIGHT_DOWN):
        return GRID_RIGHT_DOWN;
        break;
    case(RIGHT_UP):
        return GRID_RIGHT_UP;
        break;
    }
}

/* function cube_round(h): */
/*     var rx = round(h.x) */
/*     var ry = round(h.y) */
/*     var rz = round(h.z) */

/*     var x_diff = abs(rx - h.x) */
/*     var y_diff = abs(ry - h.y) */
/*     var z_diff = abs(rz - h.z) */

/*     if x_diff > y_diff and x_diff > z_diff: */
/*         rx = -ry-rz */
/*     else if y_diff > z_diff: */
/*         ry = -rx-rz */
/*     else: */
/*         rz = -rx-ry */

/*     return Cube(rx, ry, rz) */
// @TODO: Is the bug in here?
GridV round_to_gridV(float x, float y, float z)
{
    int rx = round(x);
    int ry = round(y);
    int rz = round(z);

    float x_diff = abs((float)rx - x);
    float y_diff = abs((float)ry - y);
    float z_diff = abs((float)rz - z);

    if (x_diff > y_diff &&
        x_diff > z_diff) {
        rx = -ry-rz;
    } else if (y_diff > z_diff) {
        ry = -rx-rz;
    } else {
        rz = -rx-ry;
    }

    return gridV(rx, ry, rz);
}

// @TODO: This can be a matrix multiplication if I had matrixes. Figure out what the most common
//        way to do matrix stuff in c is, and make sure it's something that can be SIMDized.
// @OPTOMIZE: If we gotta do this for tons of entities it would be good to pack all their positions
//            together. Depends on if this is the most common operation or not.
//            Watch that mike actin talk again.
/*
  Transform from grid space to screen space.
  size * [ x_x y_x z_x ] X [x]  = [x]
         [ x_y y_y z_y ]   [y]    [y]
                           [z]
 */
V2 gridV_to_pixel(HexagonGrid grid, GridV v)
{
    // Only do the math for valid points.
    assert(v.x + v.y + v.z == 0);

    float height_mul = sqrt(3);
    float width_mul = 2;

    float size = grid.hexagon_size;

    float x_x = width_mul/2.0;
    float x_y = 0;
    float y_x = -width_mul/4.0;
    float y_y = -height_mul/2.0;
    float z_x = -width_mul/4.0;
    float z_y = height_mul/2.0;

    float screen_x = (x_x * v.x + y_x * v.y + z_x * v.z) * size;
    float screen_y = (x_y * v.x + y_y * v.y + z_y * v.z) * size;

    V2 origin;
    origin.x = grid.origin_x + width_mul*size/2.0;
    origin.y = grid.origin_y - height_mul*size/2.0;

    V2 screen_coordinates = v2(screen_x, screen_y);
    screen_coordinates = v2_plus(screen_coordinates, origin);
    
    return screen_coordinates;
}

// @BUG: This isn't working, the scale seems wrong which doesn't make sense to me.
/*
  Transform from screen space to grid space. Inverse of grid to screen.
  [ x_x x_y ] X [x] / size = [x]
  [ y_x y_y ]   [y]          [y]
  [ z_x z_y ]                [z]
 */
GridV pixel_to_gridV(HexagonGrid grid, V2 pixel)
{
    float height_mul = sqrt(3);
    float width_mul = 2;

    float size = grid.hexagon_size;
    
    V2 origin;
    origin.x = grid.origin_x + width_mul*size/2.0;
    origin.y = grid.origin_y - height_mul*size/2.0;

    pixel = v2_minus(pixel, origin);

    float x_x = width_mul/2.0;
    float x_y = 0;
    float y_x = -width_mul/4.0;
    float y_y = -height_mul/2.0;
    float z_x = -width_mul/4.0;
    float z_y = height_mul/2.0;
    
    float grid_x = (pixel.x * x_x + pixel.y * x_y) / size;
    float grid_y = (pixel.x * y_x + pixel.y * y_y) / size;
    float grid_z = (pixel.x * z_x + pixel.y * z_y) / size;

    log_info("(%f,%f,%f)", grid_x, grid_y, grid_z);

    return round_to_gridV(grid_x, grid_y, grid_z);
}
