// Need to have better editors for numbers where you can drag in an arc around
// them to modify!
// Need to show editing on the scene as you are edititng!
// Need to have a play and reset button!
// THEN I need to work on the scene and make levels and stuff.

// If needed I should go back and refactor the node code too....
// Maybe I should do that before any of this...

#ifndef _game_h
#define _game_h

#include <math.h>
#include "stretchy_buffer.h"

// this is strait up taken from stb.h which I couldn't figure out how to include.
#define clamp(x,xmin,xmax)  ((x) < (xmin) ? (xmin) : (x) > (xmax) ? (xmax) : (x))

static const int X_PADDING = 12.0;
static const int Y_PADDING = 12.0;


typedef union {
    struct {float x; float y;};
    float arr[2];
} V2;


V2
v2(float x, float y)
{
    return (V2){.x = x, .y = y};
}


V2
v2_plus(const V2 i, const V2 j)
{
    return v2(i.x + j.x,
              i.y + j.y);
}


V2
v2_minus(const V2 i, const V2 j)
{
    return v2(i.x - j.x,
              i.y - j.y);
}

V2
v2_scale(const V2 v, const float f)
{
    return v2(v.x * f,
              v.y * f);
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
    return v2(nx, ny);
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

V2
bb_center(BoundingBox bb)
{
    float centerx = ((bb.bottom_right.x - bb.top_left.x) / 2)
        + bb.top_left.x;
    float centery = ((bb.bottom_right.y - bb.top_left.y) / 2)
        + bb.top_left.y;

    return v2(centerx, centery);
}

// Calculate width and height of bb
V2
bb_size(BoundingBox bb)
{
    return v2(bb.bottom_right.x - bb.top_left.x,
              bb.bottom_right.y - bb.top_left.y);
}

void
debug_square(NVGcontext* vg, float x, float y)
{
    nvgBeginPath(vg);
    nvgRect(vg, x, y, 1, 1);
    nvgFill(vg);
}

void
debug_text(NVGcontext* vg, float x, float y, int size, const char* txt)
{
    nvgSave(vg);
    nvgFontSize(vg, size);
    nvgFillColor(vg, nvgRGBf(1, 1, 1));
    nvgText(vg, x, y, txt, NULL);
    nvgRestore(vg);
}

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

// Ok, this is a problem.
// Destroying will nullify all pointers. If I compress the array then I need to update
// all pointers.
void
nodestore_destory_node(NodeStore* ns, int id)
{

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

Signal
signal_next(Signal current_signal)
{
    switch(current_signal) {
    case POS_X:
        return POS_Y;
        break;
    case POS_Y:
        return ROTATION;
        break;
    case ROTATION:
        return POS_X;
        break;
    }
}

Predicate
predicate_next(Predicate current_pred)
{
    switch(current_pred) {
    case(LT):
        return GT;
        break;
    case(GT):
        return LEQT;
        break;
    case(LEQT):
        return GEQT;
        break;
    case(GEQT):
        return EQ;
        break;
    case(EQ):
        return NEQ;
        break;
    case(NEQ):
        return LT;
        break;
    }
}

Gate
gate_next(Gate gate)
{
    switch(gate) {
    case(AND):
        return OR;
        break;
    case(OR):
        return NOT;
        break;
    case(NOT):
        return AND;
        break;
    }
}

Thruster
thruster_next(Thruster thruster)
{
    switch(thruster) {
    case(BP):
        return BS;
        break;
    case(BS):
        return SP;
        break;
    case(SP):
        return SS;
        break;
    case(SS):
        return BOOST;
        break;
    case(BOOST):
        return BP;
        break;
    }
}

BoundingBox
node_calc_bounding_box(NVGcontext* vg, const Node* node, const NodeStore* ns)
{
    // calculate the bounding box of the node. Set the bottom right point.
    if (node->type == THRUSTER) {
        BoundingBox bb = {v2(node->position.x,
                             node->position.y),
                          v2(node->position.x + 60,
                             node->position.y + 70)};
    
        return bb;
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
        nvgTextBounds(vg, node->position.x+X_PADDING,
                      node->position.y+Y_PADDING, buf, NULL, bounds);
        bb.top_left.x = bounds[0] - X_PADDING;
        bb.top_left.y = bounds[1];
        bb.bottom_right.x = bounds[2] + X_PADDING;
        bb.bottom_right.y = bounds[3] + 2*Y_PADDING;
    }
    nvgRestore(vg);
    return bb;
}

// todo(stephen): The bounds checking is shared with node_calc_bounding_box
// pull that part out.
void
draw_text_box(NVGcontext* vg, const char* txt, float x, float y)
{
    nvgSave(vg);
    {
        // Setup Text
        nvgFontSize(vg, 14);

        // Get Text Bounds
        float bounds[4];
        nvgTextBounds(vg, x+X_PADDING, y+Y_PADDING, txt, NULL, bounds);

        // Draw Background
        nvgSave(vg);
        {
            nvgBeginPath(vg);
            nvgRect(vg,
                    bounds[0] - X_PADDING,
                    bounds[1],
                    bounds[2] - bounds[0] +  2 * X_PADDING,
                    bounds[3] - bounds[1] +  2 * Y_PADDING);
            nvgFillColor(vg, nvgRGBf(0.5, 0.5, 0.5));
            nvgFill(vg);
        }
        nvgRestore(vg);

        // Draw Text
        nvgFillColor(vg, nvgRGBf(1, 1, 1));
        nvgText(vg, x+X_PADDING, y+2*Y_PADDING, txt, NULL);
    }
    nvgRestore(vg);
}


void
draw_ship(NVGcontext* vg, Thrusters thrusters, bool grayscale)
{
    nvgSave(vg);
    {
        
        if (grayscale) {
            nvgFillColor(vg, nvgRGBf(1, 1, 1));
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

// up the size of a bb by 1
BoundingBox
bb_blow_up(BoundingBox bb)
{
    bb.top_left.x -= 1;
    bb.top_left.y -= 1;
    bb.bottom_right.x += 2;
    bb.bottom_right.y += 2;
    return bb;
}

void
debug_draw_bb(NVGcontext* vg, NVGcolor color, BoundingBox bb)
{
    nvgSave(vg);
    nvgStrokeColor(vg, color);
    nvgBeginPath(vg);
    nvgRect(vg,
            bb.top_left.x,
            bb.top_left.y,
            bb.bottom_right.x-bb.top_left.x,
            bb.bottom_right.y-bb.top_left.y);
    nvgStroke(vg);
    nvgRestore(vg);
}


typedef enum { BUTTON } GUINodeType;

typedef enum {GUI_NOT_DRAGGING,
              GUI_DRAGGING_NODE,
              GUI_DRAGGING_INPUT,
              GUI_DRAGGING_OUTPUT,
              GUI_DRAGGING_CONSTANT} DraggingState;

typedef struct {
    int from_id;
    int from_input_num;
    V2 from_position; // can calculate from from_id but meh...
    V2 position;
    int value;
} DragTarget;

typedef struct {
    
} GUINode;

typedef struct {
    NVGcontext* vg;
    gg_Input input;
    DraggingState dragging_state;
    DragTarget drag_target;
} GUIState;

typedef enum { BS_NAH, BS_HOVER, BS_CLICK } ButtonState;


// draws a button there, if there was a click of it this frame returns true
bool
gui_button_with_text(GUIState gui, float x, float y, float width, float height, char* txt) {
    ButtonState bs = BS_NAH;
    
    if (bounds_contains(x, y, x+width, y+height, gui.input.mouse_x, gui.input.mouse_y)) {
        if (gui.input.click) {
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

bool
gui_button(GUIState gui, float x, float y, float width, float height) {
    return gui_button_with_text(gui, x, y, width, height, NULL);
}

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


NodeEvent
gui_nodes(GUIState* gui, NodeStore* ns)
{
    //@TODO: Replace these with variable length arrays, you do not need to allocate.
    // They also could just be parallel arrays in the gamestate and then I don't have to
    // calculate them every single frame....
    NodeBounds* bodies = NULL;
    NodeBounds* inputs = NULL;
    NodeBounds* outputs = NULL;
    NodeBounds* constants = NULL;

    // Collect collision data.
    for (int i=0; i<ns->next_id; i++) {
        if (ns->array[i].type != SIGNAL && ns->array[i].type != CONSTANT) {
            // Node body bounds
            NodeBounds body;
            body.node = &ns->array[i];
            body.bb = node_calc_bounding_box(gui->vg, &ns->array[i], ns);
            body.draw_position = ns->array[i].position;
            sb_push(bodies, body);

            // Constant bounds
            Node* lhs = nodestore_get_node_by_id(ns, ns->array[i].input.lhs);
            Node* rhs = nodestore_get_node_by_id(ns, ns->array[i].input.rhs);

            if (NULL != lhs && lhs->type == CONSTANT) {
                NodeBounds constant = {};
                constant.node = lhs;
                constant.draw_position = v2(body.draw_position.x+7,
                                            body.draw_position.y+25);
                constant.bb = (BoundingBox){constant.draw_position,
                                            v2_plus(constant.draw_position, v2(10, 10))};
                sb_push(constants, constant);
            }

            if (NULL != rhs && rhs->type == CONSTANT) {
                NodeBounds constant = {};
                constant.node = rhs;
                constant.draw_position = (V2){.x = body.bb.bottom_right.x-12,
                                              .y = body.bb.top_left.y+25};
                constant.bb = (BoundingBox){constant.draw_position,
                                            v2_plus(constant.draw_position, v2(10, 10))};
                sb_push(constants, constant);
            }

            // Output bounds
            if (ns->array[i].type != THRUSTER) {
                NodeBounds output = {};
                output.node = &ns->array[i];
                float centerx = ((body.bb.bottom_right.x - body.bb.top_left.x) / 2)
                    + body.bb.top_left.x;
                float centery = body.bb.bottom_right.y;
                // @HARDCODE
                output.bb = (BoundingBox){v2(centerx - 7.5, centery - 5),
                                          v2(centerx + 7.5, centery + 5)};
                output.draw_position = output.bb.top_left;
                sb_push(outputs, output);
            }

            // Inputs bounds
            if (ns->array[i].type != PREDICATE) {
                NodeBounds input = {};
                float centerx = ((body.bb.bottom_right.x - body.bb.top_left.x) / 2)
                    + body.bb.top_left.x;
                float centery = body.bb.top_left.y;
                
                if (ns->array[i].type == GATE &&
                    ns->array[i].gate != NOT) {
                    // 2 inputs for AND and OR gates.
                    input.node = &ns->array[i];

                    input.bb = (BoundingBox){v2(body.bb.top_left.x + 2.5, centery - 5),
                                             v2(body.bb.top_left.x + 17.5, centery + 5)};
                    input.draw_position = input.bb.top_left;
                    input.input_index = 1;
                    if (input.node->input.lhs != -1) {
                        input.input_to = nodestore_get_node_by_id(ns, input.node->input.lhs);
                    }
                    sb_push(inputs, input);

                    input.bb = (BoundingBox){v2(body.bb.bottom_right.x - 17.5, centery - 5),
                                             v2(body.bb.bottom_right.x - 2.5, centery + 5)};
                    input.draw_position = input.bb.top_left;
                    input.input_index = 2;
                    if (input.node->input.rhs != -1) {
                        input.input_to = nodestore_get_node_by_id(ns, input.node->input.rhs);
                    } else {
                        input.input_to = NULL;
                    }
                    sb_push(inputs, input);
                } else {
                    // 1 of them for NOT gates and Thrusters.
                    input.node = &ns->array[i];
                    input.bb = (BoundingBox){v2(centerx - 7.5, centery - 5),
                                             v2(centerx + 7.5, centery + 5)};
                    input.draw_position = input.bb.top_left;
                    input.input_index = 1;
                    if (input.node->parent != -1) {
                        input.input_to = nodestore_get_node_by_id(ns, input.node->parent);
                    }
                    sb_push(inputs, input);
                }
            }            
        }
    }

    // Handle current gui state.
    
    // CLICK
    // START DRAGGING
    // - CONNECTOR
    // - NODE
    // CONTINUE DRAGGING
    // END DRAGGING
    if (gui->dragging_state == GUI_NOT_DRAGGING) {
        if (gui->input.start_dragging) {
            bool found = false;

            // Check if we started dragging an output.
            for (int i=0; i<sb_count(outputs); i++) {
                if (bb_contains(outputs[i].bb,
                                gui->input.mouse_x - gui->input.mouse_xrel,
                                gui->input.mouse_y - gui->input.mouse_yrel)) {

                    gui->dragging_state = GUI_DRAGGING_OUTPUT;
                    gui->drag_target.from_id = outputs[i].node->id;
                    gui->drag_target.position = outputs[i].bb.top_left;

                    if (gui->input.mouse_motion) {
                        gui->drag_target.position.x += gui->input.mouse_xrel;
                        gui->drag_target.position.y += gui->input.mouse_yrel;
                    }

                    found = true;
                    break;
                }
            }
            
            // @COPYPASTE
            // Check if we started dragging an input.
            if (!found) {
                for (int i=0; i<sb_count(inputs); i++) {
                    if (bb_contains(inputs[i].bb,
                                    gui->input.mouse_x - gui->input.mouse_xrel,
                                    gui->input.mouse_y - gui->input.mouse_yrel)) {

                        gui->dragging_state = GUI_DRAGGING_INPUT;
                        gui->drag_target.from_id = inputs[i].node->id;
                        gui->drag_target.from_input_num = inputs[i].input_index;
                        gui->drag_target.position = inputs[i].bb.top_left;

                        if (gui->input.mouse_motion) {
                            gui->drag_target.position.x += gui->input.mouse_xrel;
                            gui->drag_target.position.y += gui->input.mouse_yrel;
                        }

                        found = true;
                        break;
                    }
                }
            }

            // @COPYPASTE
            // Check if we started dragging a constant.
            if (!found) {
                for (int i=0; i<sb_count(constants); i++) {
                    if (bb_contains(constants[i].bb,
                                    gui->input.mouse_x - gui->input.mouse_xrel,
                                    gui->input.mouse_y - gui->input.mouse_yrel)) {

                        gui->dragging_state = GUI_DRAGGING_CONSTANT;
                        gui->drag_target.from_id = constants[i].node->id;
                        gui->drag_target.position = constants[i].bb.top_left;
                        gui->drag_target.from_position = bb_center(constants[i].bb);

                        if (gui->input.mouse_motion) {
                            gui->drag_target.position.x += gui->input.mouse_xrel;
                            gui->drag_target.position.y += gui->input.mouse_yrel;
                        }

                        found = true;
                        break;
                    }
                }
            }
            
            // @COPYPASTE
            // Check if we started dragging a node.
            if (!found) {
                for (int i=0; i<sb_count(bodies); i++) {
                    if (bb_contains(bodies[i].bb,
                                    //@TODO: Maybe change this to just mouse x and y.
                                    gui->input.mouse_x - gui->input.mouse_xrel,
                                    gui->input.mouse_y - gui->input.mouse_yrel)) {
                    
                        Node* drag_node = bodies[i].node;
                        //@TODO: return move event don't mutate
                        if (gui->input.mouse_motion) {
                            drag_node->position.x += gui->input.mouse_xrel;
                            drag_node->position.y += gui->input.mouse_yrel;
                        
                            bodies[i].draw_position.x += gui->input.mouse_xrel;
                            bodies[i].draw_position.y += gui->input.mouse_yrel;
                        }
                        gui->dragging_state = GUI_DRAGGING_NODE;
                        gui->drag_target.from_id = drag_node->id;
                        found = true;
                        break;
                    }
                }
            }
        }

    } else if (gui->dragging_state == GUI_DRAGGING_NODE) {
        if (gui->input.end_dragging) {
            gui->dragging_state = GUI_NOT_DRAGGING;

        } else {
            if (gui->input.mouse_motion) {
                // Continue dragging the node.
                // @TODO: return move event don't mutate
                Node* drag_node = nodestore_get_node_by_id(ns, gui->drag_target.from_id);
                drag_node->position.x += gui->input.mouse_xrel;
                drag_node->position.y += gui->input.mouse_yrel;

                // Update draw position, could probably combine this with above and
                // only loop once.
                for (int i=0; i<sb_count(bodies); i++) {
                    if (bodies[i].node->id == drag_node->id) {
                        bodies[i].draw_position.x += gui->input.mouse_xrel;
                        bodies[i].draw_position.y += gui->input.mouse_yrel;
                        break;
                    }
                }
            }
        }
        
    } else if (gui->dragging_state == GUI_DRAGGING_INPUT ||
               gui->dragging_state == GUI_DRAGGING_OUTPUT) {
        if (gui->input.end_dragging) {
            // @TODO: Handle connecting the nodes here.

            // Connect input
            // @TODO: Check outputs too.
            if (gui->dragging_state == GUI_DRAGGING_INPUT) {
                for (int i = 0; i < sb_count(bodies); i++) {
                    // check if over a node.
                    if (bb_contains(bodies[i].bb,
                                    gui->input.mouse_x,
                                    gui->input.mouse_y) &&
                        (bodies[i].node->type == PREDICATE ||
                         bodies[i].node->type == GATE)) {
                        // Connect input to this.
                        Node* input = nodestore_get_node_by_id(ns, gui->drag_target.from_id);
                        if (gui->drag_target.from_input_num == 1) {
                            input->input.lhs = bodies[i].node->id;
                        } else {
                            input->input.rhs = bodies[i].node->id;
                        }
                        break;
                    }          
                }
            }

            gui->dragging_state = GUI_NOT_DRAGGING;
            
        } else {
            if (gui->input.mouse_motion) {
                gui->drag_target.position.x = gui->input.mouse_x;
                gui->drag_target.position.y = gui->input.mouse_y;
            }
        }

        
    } else if (gui->dragging_state == GUI_DRAGGING_CONSTANT) {
        Node* node = nodestore_get_node_by_id(ns, gui->drag_target.from_id);
        if (gui->input.end_dragging) {
            // set the constant value.
            node->constant = gui->drag_target.value;                
            gui->dragging_state = GUI_NOT_DRAGGING;
            
        } else {
            if (gui->input.mouse_motion) {
                gui->drag_target.position.x = gui->input.mouse_x;
                gui->drag_target.position.y = gui->input.mouse_y;
            }
            // Show what the number is. @TODO: Show some gui stuff which should
            // maybe be moved to another place.
            /* int the_value = gui->drag_target.value; */
            // drag target probably needs an associated value.

            // take difference, take angle to difference, probably flip a sine, translate 45dg, map to value.
            // root is the position of the bounding box for this guy...

            V2 diff = v2_minus(gui->drag_target.position, gui->drag_target.from_position);

            // need to clamp angle between -pi/2 and pi/2
            float angle = atan2(-diff.y, diff.x) + 3.14159/4.0;   // angle is between 0 and pi/2
            float scale_value = (clamp(angle, 0, 3.14159/2.0) / 3.14159/2.0) / 0.25;

            // To get the right value I need to know what the signal we are comparing it to is....
            // @TODO: need to know what signal I am comparing this to.
            float value = scale_value * 700;
            gui->drag_target.value = value;

            // Debug draw dragging line.
            nvgSave(gui->vg);
            nvgBeginPath(gui->vg);
            nvgMoveTo(gui->vg, gui->drag_target.from_position.x, gui->drag_target.from_position.y);
            nvgLineTo(gui->vg, gui->drag_target.position.x, gui->drag_target.position.y);
            nvgStrokeColor(gui->vg, nvgRGBf(0.3, 0.3, 1.0));
            nvgStroke(gui->vg);
            nvgRestore(gui->vg);

            // Debug draw angle
            char buf[64] = {'\0'};
            snprintf(buf, 64, "angle: %f, scale_value: %f, value: %f", angle, scale_value, value);

            debug_text(gui->vg, gui->drag_target.from_position.x, gui->drag_target.from_position.y - 35, 14, buf);
            
            // need the center of the bounding box we drag from.
            // need to vector from that center to the drag target.
            // need the angle from -45deg to 45deg, need to map that range to the range of the constant.
            // need to determine what the value would currently be for the thing being dragged.
            // display that value somewere, also maybe on the space scene tho this ties to that badly right now.
            // probably need a bit of rethinking how this data flows.
        }
    }

    // Draw gui elements.
    for (int i=0; i<sb_count(bodies); i++) {
        char buf[256] = {'\0'};
        NodeBounds body = bodies[i];
        Node node = *body.node;

        switch(node.type) {
        case THRUSTER: {
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

            nvgSave(gui->vg);
            {
                nvgBeginPath(gui->vg);
                nvgRect(gui->vg,
                        body.draw_position.x,
                        body.draw_position.y,
                        60, 70);
                nvgFillColor(gui->vg, nvgRGBf(0.5, 0.5, 0.5));
                nvgFill(gui->vg);

                
                nvgTranslate(gui->vg,
                             body.draw_position.x+30,
                             body.draw_position.y+35);
                draw_ship(gui->vg, thrusts, true);
            }
            nvgRestore(gui->vg);

            if (gui_button(*gui, body.bb.bottom_right.x - 10, body.bb.top_left.y + 20, 5, 5)) {
                bodies[i].node->thruster = thruster_next(node.thruster);
            }
        } break;

        case PREDICATE: {
            Node* lhs = nodestore_get_node_by_id(ns, node.input.lhs);
            Node* rhs = nodestore_get_node_by_id(ns, node.input.rhs);
            node_get_text(&node, buf, 256, lhs, rhs);
            draw_text_box(gui->vg, buf, body.draw_position.x, body.draw_position.y);

            // @TODO: all of these should return events instead of mutate!
            // Buttons!
            if (lhs->type == SIGNAL) {
                if (gui_button(*gui, body.draw_position.x+5, body.draw_position.y+20, 5, 5)) {
                    lhs->signal = signal_next(lhs->signal);
                }
            } else {
                /* if (gui_button(*gui, body.draw_position.x+5, body.draw_position.y+15, 5, 5)) { */
                /*     // toggle lhs up */
                /*     lhs->constant++; */
                /* } */
                /* if (gui_button(*gui, body.draw_position.x+5, body.draw_position.y+25, 5, 5)) { */
                /*     // toggle lhs down */
                /*     lhs->constant--; */
                /* } */
            }

            V2 center = bb_center(body.bb);
            if (gui_button(*gui, center.x-2.5, body.bb.top_left.y+25, 5, 5)) {
                // toggle pred
                bodies[i].node->predicate = predicate_next(node.predicate);
            }
            
            if (rhs->type == SIGNAL) {
                if (gui_button(*gui, body.bb.bottom_right.x-10, body.bb.top_left.y+20, 5, 5)) {
                    rhs->signal = signal_next(rhs->signal);
                }
            } else {
                // @TODO: Create a widget so you can easily edit this by clicking and dragging.
                // What state to I need, it has to be a drag target. Then I need to just know
                // the current mouse position. That tells me what the value is and how to draw
                // it.
                // I map the range it can be into a cone shape, then the value it is depends
                // on the angle the cursor is at, farther away makes it easier to pick angles.
                // Should snap at max and min values too.
                // Later also show what this value is on the main board.

                // Show cone.
                // Show line to pointer.
                // Show current value.
                // (show that value on map).
                // On let go set value.

                /* I can probably draw a clipped circle maybe? */

                // set up a drag target for this.
                
                /* if (gui_button(*gui, body.bb.bottom_right.x-10, body.bb.top_left.y+15, 5, 5)) { */
                /*     // toggle rhs up */
                /*     rhs->constant++; */
                /* } */
                /* if (gui_button(*gui, body.bb.bottom_right.x-10, body.bb.top_left.y+25, 5, 5)) { */
                /*     // toggle rhs down */
                /*     rhs->constant--; */
                /* } */
            }
        } break;

        case SIGNAL:
            break;

        case CONSTANT:
            break;

        case GATE: {
            node_get_text(&node, buf, 256, NULL, NULL);
            draw_text_box(gui->vg, buf, body.draw_position.x, body.draw_position.y);

            if (gui_button(*gui, body.bb.bottom_right.x - 10, body.bb.top_left.y + 20, 5, 5)) {
                bodies[i].node->gate = gate_next(node.gate);
            }
        } break;
        }

        // To start, we probably need to get these drawing at the bounding box.
        nvgFillColor(gui->vg, nvgRGB(255,153,0));
        debug_square(gui->vg, body.draw_position.x, body.draw_position.y);        
    }

    // Draw connection lines. Arows from output to input?
    for (int i = 0; i < sb_count(inputs); i++) {
        if (inputs[i].input_to != NULL) {
            // draw line to input_to's output.
            nvgSave(gui->vg);
            nvgBeginPath(gui->vg);

            V2 input_center = bb_center(inputs[i].bb);
            nvgMoveTo(gui->vg, input_center.x, inputs[i].bb.top_left.y);
            // @TODO: Bezier

            BoundingBox parent = node_calc_bounding_box(gui->vg, inputs[i].input_to, ns);
            V2 parent_center = bb_center(parent);
            nvgLineTo(gui->vg, parent_center.x, parent.bottom_right.y + 5);
            nvgStrokeColor(gui->vg, nvgRGBf(1.0, 1.0, 1.0));
            nvgStroke(gui->vg);
            
            nvgRestore(gui->vg);

            if (gui_button(*gui,
                           ((parent_center.x - input_center.x) / 2) + input_center.x,
                           ((parent_center.y - input_center.y) / 2) + input_center.y,
                           15, 15)) {
                // Delete the connection!
                if (inputs[i].input_index == 1) {
                    inputs[i].node->input.lhs = -1;
                } else {
                    inputs[i].node->input.rhs = -1;
                }
            }
        }
    }

    // Draw destroy buttons for nodes.
    for (int i = 0; i < sb_count(bodies); i++) {
        if (gui_button(*gui,
                   bodies[i].bb.top_left.x, bodies[i].bb.top_left.y,
                       10, 10)) {
            // @TODO: Return destroy event instead of destorying.
            // @TODO: Come back to this, destroying is hard!
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

    // Debug drawing
    // debug draw node bounding boxes.
    for (int i = 0; i < sb_count(bodies); i++) {
        NVGcolor color = nvgRGBf(0.0, 1.0, 0.0);
        debug_draw_bb(gui->vg, color, bodies[i].bb);
    }

    // debug draw constant bounding boxes.
    for (int i = 0; i < sb_count(constants); i++) {
        NVGcolor color = nvgRGBf(0.0, 1.0, 1.0);
        debug_draw_bb(gui->vg, color, constants[i].bb);
    }

    // debug draw input bounding boxes.
    for (int i = 0; i < sb_count(inputs); i++) {
        NVGcolor color = nvgRGBf(1.0, 0.0, 1.0);
        debug_draw_bb(gui->vg, color, inputs[i].bb);
    }

    // debug draw output bounding boxes.
    for (int i = 0; i < sb_count(outputs); i++) {
        NVGcolor color = nvgRGBf(1.0, 1.0, 0.0);
        debug_draw_bb(gui->vg, color, outputs[i].bb);
    }

    // debug draw drag target.
    if (gui->dragging_state == GUI_DRAGGING_INPUT ||
        gui->dragging_state == GUI_DRAGGING_OUTPUT ||
        gui->dragging_state == GUI_DRAGGING_CONSTANT) {
        NVGcolor color = nvgRGBf(0.0, 1.0, 1.0);
        nvgSave(gui->vg);
        nvgFillColor(gui->vg, color);
        nvgBeginPath(gui->vg);
        nvgCircle(gui->vg,
                  gui->drag_target.position.x,
                  gui->drag_target.position.y,
                  5.0);
        nvgFill(gui->vg);
        nvgRestore(gui->vg);
    }                

    // free memory
    sb_free(bodies);
    sb_free(constants);
    sb_free(inputs);
    sb_free(outputs);

    return (NodeEvent){.type = NE_NAH};
}

typedef struct {
    NodeStore node_store;
    Ship player_ship;
    GUIState gui;

    int drag_target;
} GameState;

#endif
