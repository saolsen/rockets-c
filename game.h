#ifndef _game_h
#define _game_h

#include <math.h>

static const int X_PADDING = 12.0;
static const int Y_PADDING = 12.0;

typedef struct {
    float x;
    float y;
} Point;

typedef struct {
    float x;
    float y;
} V2;


V2
v2_plus(const V2 v1, const V2 v2)
{
    return (V2){v1.x + v2.x,
                v1.y + v2.y};
}


V2
v2_minus(const V2 v1, const V2 v2)
{
    return (V2){v1.x - v2.x,
                v1.y - v2.y};
}


float
deg_to_rad(const float deg)
{
    return deg * M_PI / 180.0; 
}

// theta must be in radians
V2
v2_rotate(const V2 v, const float theta)
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


void
node_get_text(const Node* node, char* buffer, size_t size, Node* lhs, Node* rhs)
{
    const char* txt = NULL;    
    switch(node->type) {
    case SIGNAL: {
        switch(node->signal) {
        case POS_X:
            txt = "pos-x";
            break;
        case POS_Y:
            txt = "pos-y";
            break;
        case ROTATION:
            txt = "rotation";
            break;
        }
    } break;

    case CONSTANT: {
        char str[15] = {'\0'};
        snprintf(str, 15, "%d", node->signal);
        txt = str;
        break;
    } break;

    case PREDICATE: {
        char str[15] = {'\0'};
        size_t max_len = size -1;
        
        if (NULL != lhs) {
            node_get_text(lhs, str, 15, NULL, NULL);
            strncat(buffer, str, max_len);
            strncat(buffer, " ", max_len - strlen(buffer));
            str[0] = '\0';
        }
        
        switch(node->predicate) {
        case LT:   txt = "<";  break;
        case GT:   txt = ">";  break;
        case LEQT: txt = "<="; break;
        case GEQT: txt = ">="; break;
        case EQ:   txt = "=="; break;
        case NEQ:  txt = "<>"; break;
        }

        strncat(buffer, txt, max_len - strlen(buffer));

        if (NULL != rhs) {
            node_get_text(rhs, str, 15, NULL, NULL);
            strncat(buffer, " ", max_len - strlen(buffer));
            strncat(buffer, str, max_len);
            str[0] = '\0';
        }

        return;
        
    } break;

    case GATE: {
        switch(node->gate) {
        case AND: txt = "AND"; break;
        case OR: txt = "OR"; break;
        case NOT: txt = "NOT"; break;
        } 
    } break;
    default:
        break;
    }
    
    strcpy(buffer, txt);
}


BoundingBox
node_calc_bounding_box(NVGcontext* vg, const Node* node, const NodeStore* ns)
{
    // calculate the bounding box of the node. Set the bottom right point.
    if (node->type == THRUSTER) {
        // todo(stephen): If we draw a background box use that size instead
        // of the ship size.
        return (BoundingBox){{node->position.x - 20,
                              node->position.y - 25},
                             {node->position.x + 20,
                              node->position.y + 25}};
    }

    BoundingBox bb;
    nvgSave(vg);
    {
        Node* lhs = NULL;
        Node* rhs = NULL;

        if (node->type == PREDICATE) {
            lhs = nodestore_get_node_by_id(ns, node->input.lhs);
            rhs = nodestore_get_node_by_id(ns, node->input.rhs);
        }
        
        nvgFontSize(vg, 14);
        char buf[256] = {'\0'};
        node_get_text(node, buf, 256, lhs, rhs);
        
        float bounds[4];
        nvgTextBounds(vg, node->position.x, node->position.y, buf, NULL, bounds);
        bb.top_left.x = bounds[0] - X_PADDING;
        bb.top_left.y = bounds[1] - Y_PADDING;
        bb.bottom_right.x = bounds[2] + X_PADDING;
        bb.bottom_right.y = bounds[3] + Y_PADDING;
    }
    nvgRestore(vg);

    return bb;
}

void
draw_text_box(NVGcontext* vg, const char* txt, float x, float y)
{
    nvgSave(vg);
    {
        // Setup Text
        nvgFontSize(vg, 14);

        // Get Text Bounds
        float bounds[4];
        nvgTextBounds(vg, x, y, txt, NULL, bounds);

        // Draw Background
        nvgSave(vg);
        {
            nvgBeginPath(vg);
            nvgRect(vg,
                    bounds[0] - X_PADDING,
                    bounds[1] - Y_PADDING,
                    bounds[2] - bounds[0] + 2 * X_PADDING,
                    bounds[3] - bounds[1] + 2 * Y_PADDING);
            nvgFillColor(vg, nvgRGBf(0.5, 0.5, 0.5));
            nvgFill(vg);
        }
        nvgRestore(vg);

        // Draw Text
        nvgFillColor(vg, nvgRGBf(1, 1, 1));
        nvgText(vg, x, y, txt, NULL);
    }
    nvgRestore(vg);
}


void
draw_ship(NVGcontext* vg, Thrusters thrusters, bool grayscale)
{
    nvgSave(vg);
    {
        
        if (grayscale) {
            nvgFillColor(vg, nvgRGBf(0.5, 0.5, 0.5));
        } else {
            nvgFillColor(vg, nvgRGBf(1.0, 0.0, 0.0));
        }

        // Ship body
        nvgBeginPath(vg);
        nvgRect(vg, -10.0, -25.0, 20.0, 40.0);
        nvgFill(vg);
        
        nvgBeginPath(vg);
        nvgRect(vg, -20.0, -5.0, 15.0, 30.0);
        nvgFill(vg);
        
        nvgBeginPath(vg);
        nvgRect(vg, 5.0, -5.0, 15.0, 30.0);
        nvgFill(vg);

        // Ship thrusters
        if (grayscale) {
            nvgFillColor(vg, nvgRGBf(0.75, 0.75, 0.75));
        } else {
            nvgFillColor(vg, nvgRGBf(1.0, 1.0, 0.0));
        }

        if (thrusters.bp) {
            nvgBeginPath(vg);
            nvgRect(vg, -20.0, -25.0, 10, 10);
            nvgFill(vg);
        }

        if (thrusters.bs) {
            nvgBeginPath(vg);
            nvgRect(vg, 10.0, -25.0, 10, 10);
            nvgFill(vg);
        }

        if (thrusters.sp) {
            nvgBeginPath(vg);
            nvgRect(vg, -30.0, 15.0, 10, 10);
            nvgFill(vg);
        }

        if (thrusters.ss) {
            nvgBeginPath(vg);
            nvgRect(vg, 20.0, 15.0, 10, 10);
            nvgFill(vg);
        }

        if (thrusters.boost) {
            nvgBeginPath(vg);
            nvgRect(vg, -17.5, 25.0, 10, 10);
            nvgFill(vg);
            nvgBeginPath(vg);
            nvgRect(vg, 7.5, 25.0, 10, 10);
            nvgFill(vg);
        }
    }

    nvgRestore(vg);
}


typedef enum { BUTTON } GUINodeType;

typedef struct {
    
} GUINode;

typedef struct {
    NVGcontext* vg;
    float mouse_x;
    float mouse_y;
    bool click;
} GUIState;

typedef enum { BS_NAH, BS_HOVER, BS_CLICK } ButtonState;


// draws a button there, if there was a click of it this frame returns true
bool
gui_button(GUIState gui, float x, float y, float width, float height) {
    ButtonState bs = BS_NAH;
    
    if (bounds_contains(x, y, x+width, y+height, gui.mouse_x, gui.mouse_y)) {
        if (gui.click) {
            bs = BS_CLICK;
        } else {
            bs = BS_HOVER;
        }
    }
    
    // draw button
    nvgSave(gui.vg);
    nvgFontSize(gui.vg, 14);

    switch(bs) {
    case BS_HOVER:
        nvgFillColor(gui.vg, nvgRGBf(1.0, 0.0, 0.0));
        break;
    case BS_CLICK:
        nvgFillColor(gui.vg, nvgRGBf(0.0, 1.0, 0.0));
        break;
    case BS_NAH:
        nvgFillColor(gui.vg, nvgRGBf(0.0, 0.0, 1.0));
        break;
    };
    
    nvgBeginPath(gui.vg);
    nvgRect(gui.vg, x, y, width, height);
    nvgFill(gui.vg);

    nvgRestore(gui.vg);
    
    return (BS_CLICK == bs);
}

typedef struct {
    int id;
    NodeType type;
    BoundingBox bb;
    Point draw_position;
} NODE_INFO;

typedef enum { NE_NAH } NodeEventType;

typedef struct {
    NodeEventType type;
} NodeEvent;


NodeEvent
gui_nodes(GUIState gui, NodeStore* ns)
{
    NODE_INFO* info = malloc(ns->next_id * sizeof(NODE_INFO));

    for (int i=0; i<ns->next_id; i++) {
        info[i].id = ns->array[i].id;
        info[i].type = ns->array[i].type;
        info[i].bb = node_calc_bounding_box(gui.vg, &ns->array[i], ns);
        info[i].draw_position = ns->array[i].position;


        // draw the nodes
        char buf[256] = {'\0'};
        Node node = ns->array[i];

        switch(node.type) {
        case THRUSTER: {
            // TODO(stephen): draw bounding box
            Thrusters thrusts = {};
            switch(node.thruster) {
            case BP:
                thrusts.bp = true;
                break;
            case BS:
                thrusts.bs = true;
                break;
            case SP:
                thrusts.sp = true;
                break;
            case SS:
                thrusts.ss = true;
                break;
            case BOOST:
                thrusts.boost = true;
                break;
            }

            nvgSave(gui.vg);
            {
                nvgTranslate(gui.vg, node.position.x, node.position.y);
                draw_ship(gui.vg, thrusts, true);
            }
            nvgRestore(gui.vg);
            
            /* draw_parent_line(vg, &node, nodestore_get_node_by_id(ns, node.parent)); */
        } break;

        case PREDICATE: {

            Node* lhs = nodestore_get_node_by_id(ns, node.input.lhs);
            Node* rhs = nodestore_get_node_by_id(ns, node.input.rhs);
            node_get_text(&node, buf, 256, lhs, rhs);
            draw_text_box(gui.vg, buf, node.position.x, node.position.y);
        } break;

        case SIGNAL:
            break;

        case CONSTANT:
            break;

        case GATE: {
            node_get_text(&node, buf, 256, NULL, NULL);
            draw_text_box(gui.vg, buf, node.position.x, node.position.y);
            /* draw_parent_line(vg, &node, nodestore_get_node_by_id(ns, node.input.rhs)); */
            /* draw_parent_line(vg, &node, nodestore_get_node_by_id(ns, node.input.lhs)); */
        } break;
        }
    }
    
    // draw the nodes
    // return various possible events to be handled outside
    // Drag a node (id of the node, new position and stuff)
    // Drag from (between) node connectors.
    // Delete a node
    // Delete a connector.
    // Edit a node
       // toggle signal
       // change value (keyboard entry or slider or arrows?)
    
    free(info);
    return (NodeEvent){.type = NE_NAH};
}


typedef struct {
    NodeStore node_store;
    Ship player_ship;
    GUIState gui;

    int drag_target;
} GameState;

#endif
