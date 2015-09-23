Entity*
push_entity(GameState* state, EntityType entity_type)
{
    Entity* new_entity = NULL;
    if (state->first_free_entity) {
        new_entity = state->first_free_entity;
        state->first_free_entity = new_entity->next_entity;
    } else {
        assert(state->num_entities < state->max_entities);
        new_entity = state->entities + state->num_entities++;
    }

    new_entity->id = ++state->last_used_entity_id;
    new_entity->type = entity_type;
    new_entity->flags = 0;
    new_entity->position.tile = GRID_ZERO;
    new_entity->position.facing = UP;
    new_entity->next_position = new_entity->position;
    new_entity->thrusters = 0;
    new_entity->next_entity = NULL;

    return new_entity;
}
