#ifndef _game_h
#define _game_h

#include <math.h>

typedef struct {
    float x;
    float y;
} Point;

typedef struct {
    float x;
    float y;
} V2;

V2 v2_plus(const V2 v1, const V2 v2)
{
    return (V2){v1.x + v2.x,
                v1.y + v2.y};
}


V2 v2_minus(const V2 v1, const V2 v2)
{
    return (V2){v1.x - v2.x,
                v1.y - v2.y};
}

float deg_to_rad(const float deg)
{
    return deg * M_PI / 180.0; 
}

// theta must be in radians
V2 v2_rotate(const V2 v, const float theta)
{
    float nx = v.x * cos(theta) - v.y * sin(theta);
    float ny = v.x * sin(theta) + v.y * cos(theta);
    return (V2){nx, ny};
}

typedef struct BoundingBox {
    V2 top_left;
    V2 bottom_right;
} BoundingBox;

bool
bounds_contains(float top_leftx, float top_lefty,
                float bottom_rightx, float bottom_righty,
                float x, float y)
{
    return (x > top_leftx && x < bottom_rightx &&
            y > top_lefty && y < bottom_righty);
}


bool
bb_contains(BoundingBox bb, float x, float y)
{
    return bounds_contains(bb.top_left.x,
                           bb.top_left.y,
                           bb.bottom_right.x,
                           bb.bottom_right.y, x, y);
}


// todo(stephen): Here's a bunch more stuff that I need to do with these nodes
// for them to work with an editor.
// 1) I need to know the size of the nodes.
// 2) I need to know if the mouse is over one of the nodes.
// 3) I need to handle clicking and dragging.
// 4) I need dopdown menus.
// 5) I need scroll wheel or up down arrow stuff.
// 6) I need it to just work so I can move on to puzzles.


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
    Point position;
    int rotation;
    Thrusters thrusters;
} Ship;


typedef struct Node {
    int id;
    NodeType type;
    Point position;
    BoundingBox bb;
    // todo(stephen): Put a bounding box or dragging thing on here instead of
    // just position.

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
} Node;


typedef struct {
    Node* array;
    size_t size;
    size_t next_id;
} NodeStore;


void
nodestore_init(NodeStore* ns, size_t init_size)
{
    ns->array = (Node *)malloc(init_size * sizeof(Node));
    ns->size = init_size;
    ns->next_id = 0;
}


void
nodestore_resize(NodeStore* ns)
{
    if (ns->next_id == ns->size) {
        ns->size *= 2;
        ns->array = (Node *)realloc(ns->array, ns->size * sizeof(Node));
    }
}


Node*
nodestore_get_node_by_id(const NodeStore* ns, int id)
{
    if (id >= 0 && id < ns->next_id) {
        Node* node = &(ns->array[id]);
        return node;
    } else {
        return NULL;
    }
}


Node*
nodestore_init_new_node(NodeStore* ns, float pos_x, float pos_y)
{
    nodestore_resize(ns);

    int id = ns->next_id++;
    Node* node = &(ns->array[id]);

    node->id = (int)id;
    node->position.x = pos_x;
    node->position.y = pos_y;
    
    node->input.lhs = -1;
    node->input.rhs = -1;

    return node;
}


int
nodestore_add_gate(NodeStore* ns, float pos_x, float pos_y, Gate type)
{
    Node* node = nodestore_init_new_node(ns, pos_x, pos_y);
    node->type = GATE;
    node->gate = type;

    return node->id;
}


int
nodestore_add_predicate(NodeStore* ns, float pos_x, float pos_y, Predicate type)
{
    Node* node = nodestore_init_new_node(ns, pos_x, pos_y);
    node->type = PREDICATE;
    node->predicate = type;

    return node->id;
}


int
nodestore_add_signal(NodeStore* ns, float pos_x, float pos_y, Signal type)
{
    Node* node = nodestore_init_new_node(ns, pos_x, pos_y);
    node->type = SIGNAL;
    node->signal = type;

    return node->id;
}


int
nodestore_add_constant(NodeStore* ns, float pos_x, float pos_y, int val)
{
    Node* node = nodestore_init_new_node(ns, pos_x, pos_y);
    node->type = CONSTANT;
    node->constant = val;

    return node->id;
}


int
nodestore_add_thruster(NodeStore* ns, float pos_x, float pos_y,
                       Thruster thruster)
{
    Node* node = nodestore_init_new_node(ns, pos_x, pos_y);
    node->type = THRUSTER;
    node->thruster = thruster;
    
    return node->id;
}

// Can I build a super simple ass gui thing? I HOPE SO!
// This also means I can take some stuff out of the nodes.

// buttons, dragable stuff, connecting two things (related to dragging)
// dropdowns, toggles, etc.....

typedef enum { BUTTON } GUINodeType;

typedef struct {
    
} GUINode;

typedef struct {
    NVGcontext* vg;
    float mouse_x;
    float mouse_y;
    bool click;
} GUIState;

typedef enum { NAH, HOVER, CLICK } ButtonState;

// draws a button there, if there was a click of it this frame returns true
bool gui_button(GUIState gui, float x, float y, float width, float height) {
    ButtonState bs = NAH;
    
    if (bounds_contains(x, y, x+width, y+height, gui.mouse_x, gui.mouse_y)) {
        if (gui.click) {
            bs = CLICK;
        } else {
            bs = HOVER;
        }
    }
    
    // draw button
    nvgSave(gui.vg);
    nvgFontSize(gui.vg, 14);

    if (HOVER == bs) {
        nvgFillColor(gui.vg, nvgRGBf(1.0, 0.0, 0.0));
    } else if (CLICK == bs) {
        nvgFillColor(gui.vg, nvgRGBf(0.0, 1.0, 0.0));
    } else {
        nvgFillColor(gui.vg, nvgRGBf(0.0, 0.0, 1.0));
    }
    
    nvgBeginPath(gui.vg);
    nvgRect(gui.vg, x, y, width, height);
    nvgFill(gui.vg);

    nvgRestore(gui.vg);
    
    return (CLICK == bs);
}


typedef struct {
    NodeStore node_store;
    Ship player_ship;
    GUIState gui;

    int drag_target;
} GameState;

#endif
