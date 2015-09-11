#ifndef _rockets_grid_h
#define _rockets_grid_h

// Movement on a hexagon grid uses cubic coordinates.
/* typedef struct { */
/*     int x, y, z; */
/* } V3i; */

// Want to cover math for to and from screen space.
typedef struct {
    int rows, columns;        // Width in tiles
    float origin_x, origin_y; // Screen offset used for drawing and coordinate transform.
    int hexagon_size;         // Size of heach hexagon (in pixels for now)
} HexagonGrid;

#endif
