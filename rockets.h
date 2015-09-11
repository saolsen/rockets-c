/*
  @TODO:
  * Get grid drawing.
  * Get math for grid-screen translations working.
  * Visualize the different ways the rocket moves based on what thrusters are on to make sure
    it's right.
 */
#ifndef _game_h
#define _game_h

#define ARRAY_COUNT(x) (sizeof(x) / sizeof(x[0]))

#include "rockets_platform.h"
#include "rockets_math.h"
#include "rockets_grid.h"
#include "rockets_sim.h"

//#include "rockets_debug.h"
//#include "rockets_nodes.h"
//#include "rockets_entities.h"
//#include "rockets_levels.h"
//#include "rockets_gui.h"

/* typedef enum {RUNNING, PAUSED, WON, DIED} LevelStatus; */

/* const int MAX_OBSTICLES = 256; */

/* typedef struct { */
/*     GUIState gui; */
/*     NodeStore node_store; */

/*     // Don't know what all goes in a level or scene yet. */
/*     int current_level; */
/*     LevelStatus status; */

/*     char* DeathReason; */

/*     int num_entities; */
/*     Entity entities[128]; */
/*     Entity* first_free_entity; */
/* } GameState; */

/* // @TODO: These can go in rockets_entities.c if I make that. */
/* void entity_add_flags(Entity* entity, uint32_t flags); */
/* bool entity_has_flags_set(Entity* entity, uint32_t flags); */
/* Entity* push_entity(GameState* gamestate); */

typedef struct {
    Position ship_position;
    uint32_t ship_thrusters;
} GameState;


#endif
