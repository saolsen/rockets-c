#ifndef _rockets_sim_h
#define _rockets_sim_h
/*
  This is the new rockets simulation that uses a discrete tilemap instead of continuous space.
  It is going to take awhile to port the whole game over so I'll be working in here for a while.
 */

typedef enum {
    UP = 0,
    UP_LEFT = 1,
    DOWN_LEFT = 2,
    DOWN = 3,
    DOWN_RIGHT = 4,
    UP_RIGHT = 5
} Direction;

typedef struct {
    int x, y;
    Direction facing;
} Position;

// Positive rotation is counter clockwise.
typedef struct {
    int x, y, rotation;
} Offset;

typedef enum {
    BP    = (1 << 0), // 1
    BS    = (1 << 1), // 2
    SP    = (1 << 2), // 4
    SS    = (1 << 3), // 8
    BOOST = (1 << 4), // 16
} Thruster;

/*
  There are a lot of movement cases. Here is a rough drawing of them.

  bp  | |  bs
  sp | | | ss
     boost

  a {-1, 2, 1}, BS & BOOST, SP & BOOST
  b {1, 2, -1}, BP & BOOST, SS & BOOST

  |  |a |16|b |  |
  |26|  |  |  |d |
  |10|8 |0 |4 |5 |
  |  |2 |  |1 |  |
  |  |  |  |  |  |

  Lots of the combinations are equivalent and that happens any time you have two thrusters on
  that cancel eachother out.
 */

// @TODO: Look into using cubic coordinates for movement because that's suppossed to simplify this stuff. This doesn't work with rotations either.

// These only work for an odd tile.
// 1, 2, 4, 7, 8, 11, 13, 14,
#define OFFSET_TABLE_SIZE 32
Offset offset_table[OFFSET_TABLE_SIZE] = {
    {0,0,0},   // 0:
    {1,0,-1},  // 1: BP
    {-1,0,1},  // 2: BS
    {0,0,0},   // 3: BP & BS
    {1,1,1},   // 4: SP
    {2,0,0},   // 5: BP & SP
    {0,0,2},   // 6: BS & SP
    {1,1,1},   // 7: BP & BS & SP
    {-1,1,-1}, // 8: SS
    {0,0,-2},  // 9: BP & SS
    {-2,0,0},  // 10: BS & SS
    {-1,1,-1}, // 11: BP & BS & SS
    {0,0,0},   // 12: SP & SS
    {1,0,-1},  // 13: BP & SP & SS
    {-1,0,1},  // 14: BS & SP & SS
    {0,0,0},   // 15: BP & BS & SP & SS
    {0,2,0},   // 16: BOOST
    {1,2,-1},  // 17: BP & BOOST   // back 1
    {-1,2,1},  // 18: BS & BOOST   // back 1
    {0,2,0},   // 19: BP & BS & BOOST
    {-1,2,1},  // 20: SP & BOOST   // back 2
    {2,1,-1},  // 21: BP & SP & BOOST
    {-1,2,2},  // 22: BS & SP & BOOST @TODO: This tight turn correct? Does it feel weird? // b 1
    {-1,2,1},  // 23: BP & BS & SP & BOOST //b1
    {1,2,-1},  // 24: SS & BOOST //b1
    {1,2,-2},  // 25: BP & SS & BOOST @TODO: see 22 //b1
    {-2,1,1},  // 26: BS & SS & BOOST 
    {1,2,-1},  // 27: BP & BS & SS & BOOST //b1
    {0,2,0},   // 28: SP & SS & BOOST
    {1,2,-1},  // 29: BP & SP & SS & BOOST //b1
    {-1,2,1},  // 30: BS & SP & SS & BOOST //b1
    {0,2,0},   // 31: BP & BS & SP & SS & BOOST
};

Direction
rotate_direction(Direction direction, int rotation)
{
    int dir = (int)direction;
    dir += rotation;
    dir = dir % 6;
    if (dir < 0) {
        dir += 6;
    }
    return (Direction)dir;
}


// @TODO: handle moving when rotated.
Position
next_ship_position(Position position, uint32_t thrusters)
{
    assert(thrusters < OFFSET_TABLE_SIZE);
    
    Offset offset = offset_table[thrusters];
    position.x += offset.x;
    position.y += offset.y;
    position.facing = rotate_direction(position.facing, offset.rotation);
    
    return position;
}

// @TODO: Way to test this is just show the ship on a grid and make an x based on the position
// of the next location. Have a little rocket toggle. Lets me write the visualization part too.

#endif
