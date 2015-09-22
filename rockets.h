/*
  @TODO:
  * @NEXT: Bezier curve drawing for dragging source and connected nodes.

    - Finish node gui.
    - Node eval.
    - Levels.

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

  I need to try and standardize my naming conventions.
  * used and size, vs size and max size vs num and max_num
  * naming enums, namespacing.
  
  maybe use size and used for raw memory buffers
  and use num and max_num for typed arrays

  If I wanted to show off rockets to jeremy at strange loop how far would I have to be?

  - Nodes working again.
  - Levels working.
  - Simulation working (maybe some interpolated animation between states.)

  - For dragging maybe I can have a drag target group id. Then I can say, this thing can be dragged
    to any A target. If it's dragged to any A target, you return the target it's dragged to.

  - Simulation should be easy enough to get working. I need to do icons for the buttons but then
    they should be nice enough to use. Interpolation between states should be pretty easy.

  - For levels I can leverage a lot of what I already had.

  - Hardest part really is going to be figuring out how these sensor nodes are going to work.
  
  - Dunno if this is feasable. Maybe on the plane thursday I can do some hacking, or at work
    this week. That seems like a lot to acomplish with stuff to do each night this week.

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

bool
thrusters_on(uint32_t ship_thrusters, uint32_t check_thrusters)
{
    return ship_thrusters & check_thrusters;
}

typedef struct {
    MemoryArena persistent_store;

    GUIState* gui_state;
    NodeStore* node_store;
    
    int tick;

    // Ship stuff.
    Position ship_position;
    uint32_t ship_thrusters;
} GameState;


#endif
