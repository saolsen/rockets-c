#ifndef _rockets_levels_h
#define _rockets_levels_h

// An EntitySpec describes as much information as you need to build a real entity.
// These levelspecs are translated into full sets of entities when the level is loaded.
// See rockets_levels.c to see how these are translated into game entities.
typedef enum {ES_PLAYER_SHIP,
              ES_GOAL,
              ES_OBSTICLE} EntitySpecType;

typedef struct {
    EntitySpecType type;
    int id;
    V2 position;
    V2 size;
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

int
add_obsticle(LevelSpec* level, float x, float y, float width, float height)
{
    EntitySpec* entity = push_entity_spec(level);
    entity->type = ES_OBSTICLE;
    entity->position = v2(x, y);
    entity->size = v2(width, height);
    return entity->id;
}


// gotta go up
void level_1(LevelSpec* level)
{
    add_player_ship(level, 300, 99);
    add_goal(level, 300, 600);
}

// gotta move over
void level_2(LevelSpec* level)
{
    add_player_ship(level, 300, 99);
    add_goal(level, 500, 600);
}

// gotta move up and over
void level_3(LevelSpec* level)
{
    add_player_ship(level, 300, 99);
    add_goal(level, 500, 600);
    add_obsticle(level, 300, 600, 20, 20);
    add_obsticle(level, 500, 99, 20, 20);
}

// gotta move up and over but not at the middle
// @QUESTIONABLE: this one is only a new step if you moved over at exactly the middle, not something
// everyone will hit.
void level_4(LevelSpec* level)
{
    add_player_ship(level, 300, 99);
    add_goal(level, 500, 600);
    add_obsticle(level, 300, 600, 20, 20);
    add_obsticle(level, 500, 99, 20, 20);
    add_obsticle(level, 400, 350, 20, 20);
}

// gotta turn
void level_5(LevelSpec* level)
{
    // ship is 40 x 50 so need a 40px gap you gotta fly through
    add_player_ship(level, 100, 99);
    add_goal(level, 500, 600);

    add_obsticle(level, 300, 165, 40, 330);
    add_obsticle(level, 300, 535, 40, 330);
    
}

// @TODO: Better table here, this is annoying.
LevelSpec  get_level_spec(int level)
{
    LevelSpec spec = {};
    switch(level) {
    case(1):
        level_1(&spec);
        break;
    case(2):
        level_2(&spec);
        break;
    case(3):
        level_3(&spec);
        break;
    case(4):
        level_4(&spec);
        break;
    case(5):
        level_5(&spec);
        break;
    default:
        log_error("Error, asking for unknown level.");
        break;
    }
    return spec;
}

int num_levels = 1;

#endif
