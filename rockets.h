/*
  @TODO:
  * @NEXT
    - I have a bug in my gui pushbuffer. I need to handle alignment because not handling
      alignment is crashing everything hard.

    - If ya not ready to do the rules yet hack out the arena and use it.

    - I really need to stop procrastinating and work on the nodes and gameplay.

    - Start hacking on rules / nodes.
    - Make GUI not suck this time.
    - Set the debug stuff back up.
  
  * Get some nodes stuff hooked up.
  * Try out a flying sauser ship see how much easier that is.

  I think the nodes could have sensors that check "sectors" like the up sector and the up_left
  sector and then I can have some node combinations that do set difference or union stuff.

  Programming is hard.
  - How to really reason about the data and operations. Data oriented design etc.
    have I just not come accross stuff where this is important? Is that just for like animation
    systems and stuff?

  Maybe it would be useful to have a basic arena that I can reuse.
  I basically have an arena for the gui code, if I also end up with a pushbuffer for rendering
  then I should probably do it.
  
  So I definately don't want to inline those huge arrays into my gamestate because then the
  gamestate doesn't fit in a cache line. I need to use pointers to sections of the arena.
  
 */
#ifndef _rockets_h
#define _rockets_h

#define ARRAY_COUNT(x) (sizeof(x) / sizeof(x[0]))

#include "rockets_platform.h"
#include "rockets_arena.h"
#include "rockets_math.h"
#include "rockets_grid.h"
#include "rockets_sim.h"
#include "rockets_render.h"
#include "rockets_gui.h"
#include "rockets_nodes.h"

/* #include "rockets_debug.h" */
/* #include "rockets_entities.h" */
/* #include "rockets_levels.h" */


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


// @TODO: move back to rockets_sim
typedef enum {
    BP    = (1 << 0), // 1
    BS    = (1 << 1), // 2
    SP    = (1 << 2), // 4
    SS    = (1 << 3), // 8
    BOOST = (1 << 4), // 16
} Thruster;

bool
thrusters_on(uint32_t ship_thrusters, uint32_t check_thrusters)
{
    return ship_thrusters & check_thrusters;
}

typedef struct {
    MemoryArena persistent_store;

    GUIState* gui_state;
    /* NodeStore node_store; */
    
    int tick;

    // Ship stuff.
    /* GridV ship_position; */
    Position ship_position;
    /* Direction ship_facing; */
    uint32_t ship_thrusters;
} GameState;


#endif
