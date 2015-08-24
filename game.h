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

typedef struct Thrusters {
    bool bp;
    bool bs;
    bool sp;
    bool ss;
    bool boost;
} Thrusters;

typedef struct Ship {
    V2 position;
    int rotation;
    Thrusters thrusters;
} Ship;

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

/* typedef enum {GUI_NOT_DRAGGING, */
/*               GUI_DRAGGING_NODE, */
/*               GUI_DRAGGING_INPUT, */
/*               GUI_DRAGGING_OUTPUT, */
/*               GUI_DRAGGING_CONSTANT} DraggingState; */

typedef enum {GUI_NAH,
              GUI_DRAGGING_NODE,
              GUI_DRAGGING_INPUT,
              GUI_DRAGGING_OUTPUT,
              GUI_EDITING_NODE} GUI_State;

typedef struct {
    int id;
    int value;
} NodeEdit;

// @TODO: maybe break this out for different things we're dragging.
typedef struct {
    int from_id;
    int from_input_num;
    V2 from_position; // can calculate from from_id but meh...
    V2 position;
    int value;
} DragTarget;

/* typedef struct { */
/*     gg_Input input; */
/*     GUIState gui_state; */
/*     DragTarget drag_target; */
/* } GUIState;  */

typedef struct {
    NVGcontext* vg;
    gg_Input input;
    GUI_State state;
    DragTarget drag_target;
} GUIState;

typedef enum { BS_NAH, BS_HOVER, BS_CLICK } ButtonState;

typedef enum {RUNNING, PAUSED, WON, DIED} LevelStatus;

const int MAX_OBSTICLES = 256;

typedef struct {
    GUIState gui;
    NodeStore node_store;
    Ship player_ship;

    // Don't know what all goes in a level or scene yet.
    int current_level;
    V2 goal;
    BoundingBox obstacles[MAX_OBSTICLES];
    size_t num_obstacles;

    LevelStatus status;
    char* DeathReason;

} GameState;

#endif
