#include "rockets.h"

Position
next_ship_position(Position position, uint32_t thrusters)
{
    assert(thrusters < OFFSET_TABLE_SIZE);
    MoveOffset offset = offset_table[thrusters];

    log_info("offset: (%i,%i,%i)", offset.translation.x, offset.translation.y, offset.translation.z);
    GridV movement = gridV_rotate(offset.translation, position.facing);
    log_info("rotated: (%i,%i,%i)", movement.x, movement.y, movement.z);

    position.tile = gridV_plus(position.tile, movement);
    position.facing = rotate_direction(position.facing, offset.rotation);

    return position;
}
