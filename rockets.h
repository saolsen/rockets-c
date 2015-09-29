/*
  Prototype 2
  - 5 levels.
  - GUI that shows some feedback.
  - Interpolated animation.
  - Icons for the buttons maybe.
  - Win / Lose conditions in the simulation.

  - size/used for memory buffers, num/max_num for object arrays.

  Rockets is almost playable!
  @TODO
  - Show thruster selector on thruster node.
  - Integrate entity into sensor node. (selector and make it work)
  - Do the grid area math to support ^
  - More than 1 level.
  - Lose conditions.
  - Animation.
  
  * Try out a flying sauser ship see how much easier that is.

  @TODO, @NOTE, @INCOMPLETE, @HACK, @HARDCODE, @FRAGILE, @CLEANUP
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
#include "rockets_entities.h"
/* #include "rockets_debug.h" */
/* #include "rockets_levels.h" */

// swaps a and b if a is > b
void sort(int* a, int* b)
{
    if (*a > *b) {
        int temp = *a;
        *a = *b;
        *b = temp;
    }
}

bool
thrusters_on(uint32_t ship_thrusters, uint32_t check_thrusters)
{
    return ship_thrusters & check_thrusters;
}

typedef enum {RUNNING=0, PAUSED=1, WON=2, DIED=3} LevelStatus;

char* levelstatus_names[] = {
        "RUNNING",
        "PAUSED",
        "WON",
        "DIED",
};

typedef struct {
    MemoryArena persistent_store;

    GUIState* gui_state;
    NodeStore* node_store;
    
    float running_time;
    float next_tick;
    uint32_t calculated_thrusters;

    // Current Level
    LevelStatus level_status;
    int current_level;

    Entity* ship_entity;
    Entity* goal_entity;

    // Entities.
    int last_used_entity_id;
    Entity* entities;
    int num_entities;
    int max_entities;
    Entity* first_free_entity;

} GameState;

#endif
