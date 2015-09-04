#ifndef _game_h
#define _game_h

typedef union {
    struct {float x; float y;};
    float arr[2];
} V2;

typedef struct BoundingBox {
    V2 top_left;
    V2 bottom_right;
} BoundingBox;

// @TODO: Some day you're gonna want math, that's gonna be a lot of work.
// Rule Nodes
typedef enum { THRUSTER, PREDICATE, SIGNAL, CONSTANT, GATE } NodeType;
typedef enum { BP, BS, SP, SS, BOOST }                       Thruster;
typedef enum { LT, GT, LEQT, GEQT, EQ, NEQ }                 Predicate;
typedef enum { POS_X, POS_Y, ROTATION }                      Signal;
typedef enum { AND, OR, NOT }                                Gate;

// @TODO: Compress this to a bit array.
typedef struct Thrusters {
    bool bp;
    bool bs;
    bool sp;
    bool ss;
    bool boost;
} Thrusters;

typedef struct Node {
    int id;
    NodeType type;
    V2 position;
    BoundingBox bb;

    union {
        struct {
            int lhs;
            int rhs; } input;
        int parent;
    };
    union {
        Thruster  thruster;
        Predicate predicate;
        Signal    signal;
        int       constant;
        Gate      gate;
    };

    struct Node* next_in_hash;
} Node;


typedef struct {
    int count;
    int last_id; // last_id because we start with 1, increment before using.
    Node nodes[256];
    Node* first_free_node;

    // @NOTE: Must be power of 2 for bad hash function.
    Node* id_hash[128];

    // @TODO: Add another index for the topological sort. Makes evaluation faster.
} NodeStore;

// @NOTE: I feel great about the nodestore, I don't feel good about the UI code.
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
    float collision_area_x, collision_area_y, collision_area_width, collision_area_height;
    
} GameState;

// NAH.

#endif
