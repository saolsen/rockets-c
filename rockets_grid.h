#ifndef _rockets_grid_h
#define _rockets_grid_h

typedef enum {
    UP = 0,
    UP_LEFT = 1,
    DOWN_LEFT = 2,
    DOWN = 3,
    DOWN_RIGHT = 4,
    UP_RIGHT = 5
} Direction;

// Type to represent a vector on the grid in cubic coordinates. Could be a position or a
// direction.
typedef struct {
    int x, y, z;
} GridV;

GridV gridV(int x, int y, int z);

#define GRID_ZERO       gridV(0,0,0)
#define GRID_UP         gridV(0,1,-1)
#define GRID_UP_LEFT    gridV(-1,1,0)
#define GRID_DOWN_LEFT  gridV(-1,0,1)
#define GRID_DOWN       gridV(0,-1,1)
#define GRID_DOWN_RIGHT gridV(1,-1,0)
#define GRID_UP_RIGHT   gridV(1,0,-1)

int gridV_eq(GridV a, GridV b);
int gridV_distance(GridV a, GridV b);
int gridV_magnitude(GridV v);
GridV gridV_plus(GridV a, GridV b);
GridV gridV_scale(GridV v, float n);
GridV gridV_rotate(GridV v, int rotation);
GridV gridV_for_direction(Direction direction);

typedef struct {
    int rows, columns;        // Width in tiles
    float origin_x, origin_y; // Screen offset used for drawing and coordinate transform.
    int hexagon_size;         // Size of heach hexagon (in pixels for now)
} HexagonGrid;

// Math for translating between grid and screen space.
V2 gridV_to_pixel(HexagonGrid grid, GridV v);

// @NOTE: Here if needed. Delete if not.
/* Direction */
/* rotate_direction(Direction direction, int rotation) */
/* { */
/*     int dir = (int)direction; */
/*     dir += rotation; */
/*     dir = dir % 6; */
/*     if (dir < 0) { */
/*         dir += 6; */
/*     } */
/*     return (Direction)dir; */
/* } */
#endif
