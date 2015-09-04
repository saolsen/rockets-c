/*
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

  * Rendering
    Pull out any raw nanovg calls. Put all that behind a rendering API. Make the debug stuff part
    of debug utils and the drawing stuff abstracted so I can ditch nanovg later.

  @TODO: next steps.
  * Better debug setup. Logging, performance counters, easier debug visualizations.
    (watch casey on this stuff today)
  * Debug the collision detection code.
  * Use collisions for gameplay stuff.
 */
#ifndef _game_h
#define _game_h

#define ARRAY_COUNT(x) (sizeof(x) / sizeof(x[0]))

#include "rockets_platform.h"
#include "rockets_math.h"
#include "rockets_nodes.h"


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

// Entities
// @TODO: rockets_entities

typedef enum {EntityType_NAH,
              EntityType_SHIP,
              EntityType_BOUNDRY,
              EntityType_GOAL} EntityType;

typedef enum {EntityFlag_COLLIDES = (1 << 0)} EntityFlag;

// Start with a special case of just having all collision geometry be rectangles.
// For now going to have rotation be about the entitiy's position. Makes math easy
// and seems to make sense for things that are going to move around.

// @NOTE: This is basically the same as bounding box but the names are different because this is
// a different way to describe a rectangle.
typedef struct {
    union {
        struct {float x, y;};
        V2 offset;
    };
    union {
        struct {float width, height; };
        V2 size;
    };
} CollisionRect;


// How do I determine entity equality? Just pointer equality?
typedef struct entity_ {
    EntityType type;
    uint32_t flags;

    V2 position;
    int rotation;
    Thrusters thrusters;

    V2 velocity;

    // @TODO: I think I should probably use a memory area for this instead of allocating
    // 10 slots for every entity. Many will just have 1 piece and lots won't have any.
    // If I have a memory area I can track memory usage in it too.
    // Also could let me pack all collision pieces in a single array which might be dope
    // if I want to SIMD my collision checking.
    int num_collision_pieces;
    CollisionRect collision_pieces[10];

    struct entity_* next_entity;
} Entity;

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

    // This is just the last collision area checked and a crappy way to debug.
    // @TODO: Way better debug tools.
    float collision_area_x, collision_area_y, collision_area_width, collision_area_height;
    
} GameState;

// NAH.

#endif
