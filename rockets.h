/*
  @TODO: @IMPORTANT: Known Bugs, fix these ASAP.
  * The collision detection stuff isn't working properly.

  @TODO: next steps.
  * Better debug setup. Logging, performance counters, easier debug visualizations.
    (watch casey on this stuff today)
  * Debug the collision detection code.
  * Use collisions for gameplay stuff.
  * Start really making some levels and play with the game mechanics. Any platform work before having
    a viable game and levels is just wasted.

  What do I need for a minimum viable level system.
  - Track what level you on.
  - Build levels from a simple description.
  - Compile description into scene entities.

  Don't make a website or design a ui or anything until you got a bunch of fun levels.
  If I make an actualy fun game, I can actually make money on it if I push it.
  But if the game isn't fun, it's never gonna happen.

  @TODO: Stuff I think I should do.
  * Debug code.
  I really want more debug code. I was leaning on it heavily for the collision detection stuff. I 
  think I could just add some helper functions for drawing to the space scen which would help but
  I also really want performance counters and a log and ways to visualize it. I should maybe wait
  until casey is done with his stuff and see what of that I can use. 
  I should also probably do the live code reloading stuff and I maybe want a level editor.

  * Platform
    I need to figure out my cpu/battery issue. I think I'm either not letting the process sleep
    while waiting for vsync or something else bad. Need to look into sdl resources to see what's up.

  * Game
    Gonna add some obsticles and things to try and fly around and make a few harder levels and see
    what it's like to play the game with these nodes and what else I should add or change.

    There are more bugs in the collision detection stuff and I'm going to have to write a more
    advanced version that handles rotations too so maybe I should just invest in debug tools a
    bit more.

    I really really need to start making levels and playing with the mechanics. I need to figure
    out if there is enough depth in the mechanic to make meaningful puzzles and a fun game. This
    is the real make or break point for the game, it has to have good puzzles. After that I also
    need a good story.

  * Rendering
    Pull out any raw nanovg calls. Put all that behind a rendering API. Make the debug stuff part
    of debug utils and the drawing stuff abstracted so I can ditch nanovg later.
    Real renderer is probably going to be a lot more complex. I want a nice looking game.
    For now this vector stuff works fine.

  Porting
  * Once I really have a game it would be great to get it running on a lot of platforms.
  - This probably means abstracting the rendering api which would also probably mean writing the
    rendering from scratch, casey style but adding hardware acceleration. Can port this to any tablet
    that has a rendering api and those that don't even.
  
  I'll keep saying and believing this. HMH is the absolute best thing I've ever watched.
  Just keep watching it!
    

 */
#ifndef _game_h
#define _game_h

#define ARRAY_COUNT(x) (sizeof(x) / sizeof(x[0]))

#include "rockets_platform.h"
#include "rockets_math.h"
#include "rockets_debug.h"
#include "rockets_nodes.h"
#include "rockets_entities.h"
#include "rockets_levels.h"

// GUI stuff
// @TODO: rockets_gui

typedef struct {
    Node* node;
    BoundingBox bb;
    V2 draw_position;
    int input_index;
    Node* input_to;
} NodeBounds;

typedef enum { NE_NAH } NodeEventType;

typedef struct {
    NodeEventType type;
} NodeEvent;

typedef enum {GUI_NAH,
              GUI_DRAGGING_NODE,
              GUI_DRAGGING_INPUT,
              GUI_DRAGGING_OUTPUT,
              GUI_DRAGGING_SLIDER} GUI_State;

typedef enum {DT_NAH,
              DT_BODY,
              DT_OUTPUT,
              DT_INPUT,
              DT_SLIDER} DragTarget_Type;

typedef struct {
    DragTarget_Type type;
    int from_id;
    int from_input_num;
    V2 from_position; // can calculate from from_id but meh...
    V2 position;
    int value;
} DragTarget;

typedef struct {
    NVGcontext* vg;
    gg_Input input;
    GUI_State state;
    DragTarget drag_target;
} GUIState;

typedef enum { BS_NAH, BS_HOVER, BS_CLICK } ButtonState;

typedef struct entity_pointer_ {
    Entity* entity;
    struct entity_pointer_* next_entity;
} EntityPointer;

typedef enum {RUNNING, PAUSED, WON, DIED} LevelStatus;

const int MAX_OBSTICLES = 256;

typedef struct {
    GUIState gui;
    NodeStore node_store;

    // Don't know what all goes in a level or scene yet.
    int current_level;
    LevelStatus status;

    char* DeathReason;

    int num_entities;
    Entity entities[128];
    Entity* first_free_entity;
} GameState;

// @TODO: These can go in rockets_entities.c if I make that.
void entity_add_flags(Entity* entity, uint32_t flags);
bool entity_has_flags_set(Entity* entity, uint32_t flags);
Entity* push_entity(GameState* gamestate);


#endif
