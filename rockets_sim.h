#ifndef _rockets_sim_h
#define _rockets_sim_h
/*
  @TODO: This would be so nice to be able to calculate from the original grid movement primitives
  at compile time but c won't let you do that. Maybe consider writing another small program that outputs
  this table.
*/

// More "primitives" because I can't calculate them at compile time.
#define GRID_RIGHT_2           (GridV){ 2,-1,-1}
#define GRID_LEFT_2            (GridV){-2, 1, 1}
#define GRID_UP_2              (GridV){ 0, 2,-2}
#define GRID_UP_RIGHT_UP       (GridV){ 1, 1,-2}
#define GRID_UP_LEFT_UP        (GridV){-1, 2,-1}
#define GRID_RIGHT_UP_RIGHT_UP (GridV){ 2, 0,-2}
#define GRID_LEFT_UP_LEFT_UP   (GridV){-2, 2, 0}

// Positive rotation is counter clockwise.
typedef struct {
    GridV translation;
    int rotation;
} MoveOffset;

// Movements of the ship based on which thrusters are on.
#define OFFSET_TABLE_SIZE 32
MoveOffset offset_table[OFFSET_TABLE_SIZE] = {
//  Translation        rotation
    {GRID_ZERO,               0}, // 0:  No Thrusters.
    {GRID_RIGHT_DOWN,        -1}, // 1:  BP
    {GRID_LEFT_DOWN,          1}, // 2:  BS
    {GRID_ZERO,               0}, // 3:  BP & BS
    {GRID_RIGHT_UP,           1}, // 4:  SP
    {GRID_RIGHT_2,            0}, // 5:  BP & SP
    {GRID_ZERO,               0}, // 6:  BS & SP
    {GRID_RIGHT_UP,           1}, // 7:  BP & BS & SP
    {GRID_LEFT_UP,           -1}, // 8:  SS
    {GRID_ZERO,              -2}, // 9:  BP & SS
    {GRID_LEFT_2,             0}, // 10: BS & SS
    {GRID_LEFT_UP,           -1}, // 11: BP & BS & SS
    {GRID_ZERO,               0}, // 12: SP & SS
    {GRID_RIGHT_DOWN,        -1}, // 13: BP & SP & SS
    {GRID_LEFT_DOWN,          1}, // 14: BS & SP & SS
    {GRID_ZERO,               0}, // 15: BP & BS & SP & SS
    {GRID_UP_2,               0}, // 16: BOOST
    {GRID_UP_RIGHT_UP,       -1}, // 17: BP & BOOST
    {GRID_UP_LEFT_UP,         1}, // 18: BS & BOOST
    {GRID_UP_2,               0}, // 19: BP & BS & BOOST
    {GRID_UP_LEFT_UP,         1}, // 20: SP & BOOST
    {GRID_RIGHT_UP_RIGHT_UP, -1}, // 21: BP & SP & BOOST
    {GRID_UP_LEFT_UP,         2}, // 22: BS & SP & BOOST
    {GRID_UP_LEFT_UP,         1}, // 23: BP & BS & SP & BOOST
    {GRID_UP_RIGHT_UP,       -1}, // 24: SS & BOOST
    {GRID_UP_RIGHT_UP,       -2}, // 25: BP & SS & BOOST
    {GRID_LEFT_UP_LEFT_UP,    1}, // 26: BS & SS & BOOST
    {GRID_UP_RIGHT_UP,       -1}, // 27: BP & BS & SS & BOOST
    {GRID_UP_2,               0}, // 28: SP & SS & BOOST
    {GRID_UP_RIGHT_UP,       -1}, // 29: BP & SP & SS & BOOST
    {GRID_UP_LEFT_UP,         1}, // 30: BS & SP & SS & BOOST
    {GRID_UP_2,               0}, // 31: BP & BS & SP & SS & BOOST
};

typedef struct {
    GridV tile;
    Direction facing;
} Position;

Position next_ship_position(Position position, uint32_t thrusters);

#endif
