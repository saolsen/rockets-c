Position
next_ship_position(Position position, uint32_t thrusters)
{
    assert(thrusters < OFFSET_TABLE_SIZE);
   
    MoveOffset offset = offset_table[thrusters];

    GridV movement = gridV_rotate(offset.translation, position.facing);

    position.tile = gridV_plus(position.tile, movement);
    position.facing = rotate_direction(position.facing, offset.rotation);

    return position;
}
