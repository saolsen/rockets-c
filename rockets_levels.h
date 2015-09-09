#ifndef _rockets_levels_h
#define _rockets_levels_h

// An EntitySpec describes as much information as you need to build a real entity.
// These levelspecs are translated into full sets of entities when the level is loaded.
// See rockets_levels.c to see how these are translated into game entities.
typedef enum {ES_PLAYER_SHIP,
              ES_GOAL} EntitySpecType;

typedef struct {
    EntitySpecType type;
    int id;
    union {
        V2 position;
    };
} EntitySpec;

typedef struct {
    int num_entity_specs;
    EntitySpec entity_specs[256];
} LevelSpec;

EntitySpec* push_entity_spec(LevelSpec* level)
{
    EntitySpec* entity = &level->entity_specs[level->num_entity_specs++];
    entity->id = level->num_entity_specs;
    return entity;
}

// Helpers for building the entityspecs. They return the "entity_spec_id" which is used for linking
// entities together.
int
add_player_ship(LevelSpec* level, float x, float y)
{
    EntitySpec* entity = push_entity_spec(level);
    entity->type = ES_PLAYER_SHIP;
    entity->position = v2(x, y);
    return entity->id;
}

int
add_goal(LevelSpec* level, float x, float y)
{
    EntitySpec* entity = push_entity_spec(level);
    entity->type = ES_GOAL;
    entity->position = v2(x, y);
    return entity->id;
}

void level_1(LevelSpec* level)
{
    add_player_ship(level, 300, 99);
    add_goal(level, 500, 600);
}

LevelSpec  get_level_spec(int level)
{
    LevelSpec spec = {};
    switch(level) {
    case(1):
        level_1(&spec);
        break;
    default:
        log_error("Error, asking for unknown level.");
        break;
    }
    return spec;
}

#endif
