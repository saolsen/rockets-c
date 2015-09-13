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
V2 gridV_to_pixel(HexagonGrid grid, GridV v)
{
    // Only do the math for valid points.
    assert(v.x + v.y + v.z == 0);

    float q = v.x;
    float r = v.z;

    V2 origin;
    origin.x = grid.origin_x + 2*grid.hexagon_size/2.0;
    origin.y = grid.origin_y - sqrt(3)*grid.hexagon_size/2.0;

    float screen_x = q * 3.0/2.0 * grid.hexagon_size;
    float screen_y = (r + q/2.0) * sqrt(3) * grid.hexagon_size;

    V2 screen_coordinates = v2(screen_x, screen_y);
    screen_coordinates = v2_plus(screen_coordinates, origin);
    
    return screen_coordinates;
}

// @OPTOMIZE: Again if I'm doing lots of these I should pack them and have this function take
//            an array so I'm not calculating all the extra stuff in the inner loop.
GridV pixel_to_gridV(HexagonGrid grid, V2 pixel)
{   
    V2 origin;
    origin.x = grid.origin_x + 2*grid.hexagon_size/2.0;
    origin.y = grid.origin_y - sqrt(3)*grid.hexagon_size/2.0;

    pixel = v2_minus(pixel, origin);

    float q = pixel.x * 2.0/3.0 / grid.hexagon_size;
    float r = (-pixel.x / 3.0 + sqrt(3)/3.0 * pixel.y) / grid.hexagon_size;

    float grid_x = q;
    float grid_z = r;
    float grid_y = -grid_x-grid_z;

    return round_to_gridV(grid_x, grid_y, grid_z);
}
