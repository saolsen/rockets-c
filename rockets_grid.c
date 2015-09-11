// @TODO: drop this include because it's a unity build.
#include "rockets_grid.h"

int abs(int i)
{
    return i < 0 ? -i : i;
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
    return abs(a.x - b.x) + abs(a.y - b.y) + abs(a.z - b.z);
}

int
gridV_magnitude(GridV v)
{
    return gridV_distance(GRID_ZERO, v);
}

// @NOTE: This takes a float so you can do stuff like 0.5;
// You'll still get back ints so don't mess yourself up.
GridV
gridV_scale(GridV v, float n)
{
    v.x = (int)(v.x * n);
    v.y = (int)(v.y * n);
    v.z = (int)(v.z * n);
    return v;
}

// Rotate a grid vector counter clockwise around the hexagon. Rotation must be an int (1-5).
// @TODO: This is super hacky, there's probably a real math way to do it.
GridV
gridV_rotate(GridV v, int rotation)
{
    int mag = gridV_magnitude(v);
    v = gridV_scale(v, 1.0/(float)mag);

    GridV new_v;
    if (gridV_eq(v, GRID_UP)) {
        new_v = GRID_UP_LEFT;
    } else if (gridV_eq(v, GRID_UP_LEFT)) {
        new_v = GRID_DOWN_LEFT;
    } else if (gridV_eq(v, GRID_DOWN_LEFT)) {
        new_v = GRID_DOWN;
    } else if (gridV_eq(v, GRID_DOWN)) {
        new_v = GRID_DOWN_RIGHT;
    } else if (gridV_eq(v, GRID_DOWN_RIGHT)) {
        new_v = GRID_UP_RIGHT;
    } else if (gridV_eq(v, GRID_UP_RIGHT)) {
        new_v = GRID_UP;
    }

    return gridV_scale(new_v, mag);
}

// Get the unit (1 space) grid vector for a given direction.
GridV
gridV_for_direction(Direction direction)
{
    switch(direction) {
    case(UP):
        return GRID_UP;
        break;
    case(UP_LEFT):
        return GRID_UP_LEFT;
        break;
    case(DOWN_LEFT):
        return GRID_DOWN_LEFT;
        break;
    case(DOWN):
        return GRID_DOWN;
        break;
    case(DOWN_RIGHT):
        return GRID_DOWN_RIGHT;
        break;
    case(UP_RIGHT):
        return GRID_UP_RIGHT;
        break;
    }
}


// Test this shit outside of the game for a sec.
#include <stdlib.h>
#include <stdio.h>

int main() {
    printf("Hello from rockets_grid\n");
}
