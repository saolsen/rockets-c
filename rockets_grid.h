#ifndef _rockets_grid_h
#define _rockets_grid_h

typedef enum {
    UP = 0,
    LEFT_UP = 1,
    LEFT_DOWN = 2,
    DOWN = 3,
    RIGHT_DOWN = 4,
    RIGHT_UP = 5
} Direction;

Direction rotate_direction(Direction direction, int rotation);

// Type to represent a vector on the grid in cubic coordinates. Could be a position or a
// direction.
typedef struct {
    int x, y, z;
} GridV;

#define GRID_ZERO       (GridV){ 0, 0, 0}
#define GRID_UP         (GridV){ 0, 1,-1}
#define GRID_LEFT_UP    (GridV){-1, 1, 0}
#define GRID_LEFT_DOWN  (GridV){-1, 0, 1}
#define GRID_DOWN       (GridV){ 0,-1, 1}
#define GRID_RIGHT_DOWN (GridV){ 1,-1, 0}
#define GRID_RIGHT_UP   (GridV){ 1, 0,-1}

GridV gridV(int x, int y, int z);
int gridV_eq(GridV a, GridV b);
int gridV_distance(GridV a, GridV b);
int gridV_magnitude(GridV v);
GridV gridV_plus(GridV a, GridV b);
GridV gridV_minus(GridV a, GridV b);
GridV gridV_scale(GridV v, float n);
GridV gridV_rotate(GridV v, int rotation);
GridV gridV_for_direction(Direction direction);
GridV round_to_gridV(float x, float y, float z);

typedef struct {
    int rows, columns;        // Width in tiles
    float origin_x, origin_y; // Screen offset used for drawing and coordinate transform.
    int hexagon_size;         // Size of heach hexagon (in pixels for now)
} HexagonGrid;

bool point_on_grid(HexagonGrid grid, GridV v);

// Math for translating between grid and screen space.
V2 gridV_to_pixel(HexagonGrid grid, GridV v);
GridV pixel_to_gridV(HexagonGrid grid, V2 pixel);

// A set of tiles in the grid. Should be able to do some setlike things on them.
typedef struct {
    HexagonGrid grid;
    GridV position;
    Direction direction;
} TileSet;

#define TILESET_EMPTY (TileSet){}

/* // Set of all the tiles on the grid. */
/* TileSet tileset_gridbounds(HexagonGrid grid); */
/* // Set of all the tiles on the grid in a direction from a point. */
/* // 0,0,0, RIGHT_UP would be all the tiles on the grid except 0,0,0, since 0,0,0 is the bottom left corner. */

/* // @HACK to get started. */
TileSet tileset_above(HexagonGrid grid, GridV position, Direction direction)
{
    return (TileSet){.grid = grid, .position = position, .direction = direction};
}

/* // Check if a tile is in a TileSet. */
/* bool tileset_contains_tile(TileSet tile_set, GridV pos); */
/* // Integer length from origin to position in direction. */
/* int grid_distance(GridV origin, Direction direction, GridV position); */



#endif
