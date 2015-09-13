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

// @TODO: This can be a matrix multiplication if I had matrixes.
// @OPTOMIZE: If we gotta do this for tons of entities it would be good to pack all their positions
//            together. Depends on if this is the most common operation or not.
//            Watch that mike actin talk again.
/*
  [x, y, z] * [ x_x x_y ] = [x, y]
              [ y_x y_y ]
              [ z_x z_y ]

 */
V2 gridV_to_pixel(HexagonGrid grid, GridV v)
{
    // Only do the math for valid points.
    assert(v.x + v.y + v.z == 0);

    float tile_width = grid.hexagon_size * 2;
    float tile_height = sqrt(3)/2.0 * tile_width;

    

    float x_x = tile_width/2.0;
    float x_y = 0;
    float y_x = -tile_width/4.0;
    float y_y = -tile_height/2.0;
    float z_x = -tile_width/4.0;
    float z_y = tile_height/2.0;
    
    V2 x_scale = v2(x_x, x_y);
    V2 y_scale = v2(y_x, y_y);
    V2 z_scale = v2(z_x, z_y);

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

// @BUG: This isn't working
/*
  [x, y] * [ x_x y_x z_x ] = [x, y, z]
           [ x_y y_y z_y ]


 */
GridV pixel_to_gridV(HexagonGrid grid, V2 pixel)
{
    float tile_width = grid.hexagon_size * 2;
    float tile_height = sqrt(3)/2.0 * tile_width;
    
    V2 origin;
    origin.x = grid.origin_x + tile_width/2.0;
    origin.y = grid.origin_y - tile_height/2.0;

    pixel.x = pixel.x - origin.x;
    pixel.y = -pixel.y + origin.y; // is this right?

    float x_x = tile_width/2.0;
    float x_y = 0;
    float y_x = -tile_width/4.0;
    float y_y = -tile_height/2.0;
    float z_x = -tile_width/4.0;
    float z_y = tile_height/2.0;
    
    float grid_x = pixel.x * x_x + pixel.y * x_y;
    float grid_y = pixel.x * y_x + pixel.y * y_y;
    float grid_z = pixel.x * z_x + pixel.y * z_y;

    /* log_info("(%f,%f,%f)", grid_x, grid_y, grid_z); */

    return round_to_gridV(grid_x, grid_y, grid_z);
}
