// @NOTE: This is just so flymake doesn't get mad.
#include "rockets.h"

// Loads the entites for a given level.
// @TODO: Handle boarders.
void load_level(GameState* state, int level_number)
{
    memset(state->entities, 0, sizeof(Entity) * ARRAY_COUNT(state->entities));
    state->num_entities = 0;
    state->first_free_entity = NULL;
    state->current_level = level_number;

    LevelSpec spec = get_level_spec(level_number);

    for (int i = 0; i < spec.num_entity_specs; i++) {
        EntitySpec entity = spec.entity_specs[i];

        switch(entity.type) {
        case(ES_PLAYER_SHIP): {
            Entity* ship = push_entity(state);
            ship->type = EntityType_SHIP;
            ship->position = entity.position;
            ship->rotation = 0;

            entity_add_flags(ship, EntityFlag_COLLIDES);

            // @NOTE: ship is 40 x 50
            CollisionRect* s1 = &ship->collision_pieces[ship->num_collision_pieces++];
            s1->offset = v2(-10, -15);
            s1->size = v2(20, 40);
            CollisionRect* s2 = &ship->collision_pieces[ship->num_collision_pieces++];
            s2->offset = v2(-20, -25);
            s2->size = v2(15, 30);
            CollisionRect* s3 = &ship->collision_pieces[ship->num_collision_pieces++];
            s3->offset = v2(5, -25);
            s3->size = v2(15, 30);
            
        } break;
        case(ES_GOAL): {
            Entity* goal = push_entity(state);
            goal->type = EntityType_GOAL;
            goal->position = entity.position;
            goal->rotation = 0;

            entity_add_flags(goal, EntityFlag_COLLIDES);

            CollisionRect* g = &goal->collision_pieces[goal->num_collision_pieces++];
            g->offset = v2(-10, -10);
            g->size = v2(20, 20);
        } break;

        case(ES_OBSTICLE): {
            Entity* obsticle = push_entity(state);
            obsticle->type = EntityType_BOUNDRY;
            obsticle->position = entity.position;
            obsticle->rotation = 0;

            entity_add_flags(obsticle, EntityFlag_COLLIDES);

            CollisionRect* o = &obsticle->collision_pieces[obsticle->num_collision_pieces++];
            o->offset = v2(-entity.size.x/2, -entity.size.y/2);
            o->size = entity.size;
        }
        }
    }
}
