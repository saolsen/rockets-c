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

// @TODO: This can be a matrix multiplication if I had matrixes.
// @OPTOMIZE: If we gotta do this for tons of entities it would be good to pack all their positions
//            together. Depends on if this is the most common operation or not.
//            Watch that mike actin talk again.
V2 gridV_to_pixel(HexagonGrid grid, GridV v)
{
    // Only do the math for valid points.
    assert(v.x + v.y + v.z == 0);

    // @TODO: make sure the point is on the grid.

    float tile_width = grid.hexagon_size * 2;
    float tile_height = sqrt(3)/2.0 * tile_width;
    
    V2 x_scale = v2(tile_width/2.0, 0);
    V2 y_scale = v2(-tile_width/4.0, -tile_height/2.0);
    V2 z_scale = v2(-tile_width/4.0, tile_height/2.0);

    V2 screen_coordinates = v2(0,0);
    screen_coordinates = v2_plus(screen_coordinates, v2_scale(x_scale, v.x));
    screen_coordinates = v2_plus(screen_coordinates, v2_scale(y_scale, v.y));
    screen_coordinates = v2_plus(screen_coordinates, v2_scale(z_scale, v.z));

    V2 origin;
    origin.x = grid.origin_x + tile_width/2.0;
    origin.y = grid.origin_y - tile_height/2.0;

    screen_coordinates = v2_plus(screen_coordinates, origin);
    
    return screen_coordinates;
}
