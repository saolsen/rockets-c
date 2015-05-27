#ifndef _game_h
#define _game_h

#include <math.h>
#include "stretchy_buffer.h"

static const int X_PADDING = 12.0;
static const int Y_PADDING = 12.0;

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

V2
v2_scale(const V2 v, const float f)
{
    return (V2){v.x * f,
                v.y * f};
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

void
debug_square(NVGcontext* vg, float x, float y)
{
    nvgBeginPath(vg);
    nvgRect(vg, x, y, 1, 1);
    nvgFill(vg);
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
    V2 position;
    int rotation;
    Thrusters thrusters;
} Ship;

typedef struct Node {
    int id;
    NodeType type;
    V2 position;
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
        BoundingBox bb = {{node->position.x,
                           node->position.y},
                          {node->position.x + 60,
                           node->position.y + 70}};
    
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

// This looks right now but I'm not sure why exactly. Need to revisit later.
// todo(stephen): Make bounding boxes line up with this.
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
              GUI_DRAGGING_OUTPUT} DraggingState;

typedef struct {
    int from_id;
    int from_input_num;
    V2 position;
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
gui_button(GUIState gui, float x, float y, float width, float height) {
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

typedef struct {
    Node* node;
    BoundingBox bb;
    V2 draw_position;
    int InputIndex;
} NodeBounds;

typedef enum { NE_NAH } NodeEventType;

typedef struct {
    NodeEventType type;
} NodeEvent;

// Other types of events that I'm going to need.
// * Dragging things into other things.
// * Sub buttons on nodes.
//

// Need to collect the things I need.
// Bounding boxes for nodes.
// Bounding boxes for inputs.
// Bounding boxes for output.
// Bounding boxes for thruster toggles.
// Bounding boxes for destroy buttons.
// Bounding boxes for buttons.

// Current status (dragging, clicking, nothing)
// If dragging, handle dragging what into what.
// If clicking, what are you clicking.

// Keep track over scenes. (Dragging state.)

// Update (position of dragging, events for creation, deletion and editing edges)

NodeEvent
gui_nodes(GUIState* gui, NodeStore* ns)
{
    NodeBounds* bodies = NULL;
    /* NodeBounds* inputs = NULL; */
    NodeBounds* outputs = NULL;

    // Collect collision data.
    for (int i=0; i<ns->next_id; i++) {
        if (ns->array[i].type != SIGNAL && ns->array[i].type != CONSTANT) {
            // Node body bounds
            NodeBounds body;
            body.node = &ns->array[i];
            body.bb = node_calc_bounding_box(gui->vg, &ns->array[i], ns);
            body.draw_position = ns->array[i].position;
            sb_push(bodies, body);

            // Output bounds
            NodeBounds output = {};
            output.node = &ns->array[i];
            float centerx = ((body.bb.bottom_right.x - body.bb.top_left.x) / 2)
                + body.bb.top_left.x;
            float centery = body.bb.bottom_right.y;
            // @HARDCODE
            output.bb = (BoundingBox){{centerx - 7.5, centery - 5},
                                      {centerx + 7.5, centery + 5}};
            output.draw_position = (V2){.x = output.bb.top_left.x,
                                        .y = output.bb.top_left.y};
            sb_push(outputs, output);

            // Inputs bounds
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

                    gui->drag_target.position.x += gui->input.mouse_xrel;
                    gui->drag_target.position.y += gui->input.mouse_yrel;

                    found = true;
                    break;
                }
            }
            // Check if we started dragging an input.
            
            // Check if we started dragging a node.
            if (!found) {
                for (int i=0; i<sb_count(bodies); i++) {
                    if (bb_contains(bodies[i].bb,
                                    //@TODO: Maybe change this to just mouse x and y.
                                    gui->input.mouse_x - gui->input.mouse_xrel,
                                    gui->input.mouse_y - gui->input.mouse_yrel)) {
                    
                        Node* drag_node = bodies[i].node;
                        //@TODO: return move event don't mutate
                        drag_node->position.x += gui->input.mouse_xrel;
                        drag_node->position.y += gui->input.mouse_yrel;

                        bodies[i].draw_position.x += gui->input.mouse_xrel;
                        bodies[i].draw_position.y += gui->input.mouse_yrel;

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
            gui->dragging_state = GUI_NOT_DRAGGING;
            
        } else {
            if (gui->input.mouse_motion) {
                gui->drag_target.position.x += gui->input.mouse_xrel;
                gui->drag_target.position.y += gui->input.mouse_yrel;
            }
        }
        
        // show the input being dragged
        // update the drag position for this frame
        // If over an output or a node
           // if end_dragging, fire connection event.
           // else show hilight of node to be connected to.
        // show the arrow of the input being dragged

        // for output
        // very similar to above except you search nodes and inputs, toggle
        // between seperate inputs based on which is closer, this could be complex.
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
            
            /* draw_parent_line(vg, &node, nodestore_get_node_by_id(ns, node.parent)); */
        } break;

        case PREDICATE: {

            Node* lhs = nodestore_get_node_by_id(ns, node.input.lhs);
            Node* rhs = nodestore_get_node_by_id(ns, node.input.rhs);
            node_get_text(&node, buf, 256, lhs, rhs);
            draw_text_box(gui->vg, buf, body.draw_position.x, body.draw_position.y);
        } break;

        case SIGNAL:
            break;

        case CONSTANT:
            break;

        case GATE: {
            node_get_text(&node, buf, 256, NULL, NULL);
            draw_text_box(gui->vg, buf, body.draw_position.x, body.draw_position.y);
            /* draw_parent_line(vg, &node, nodestore_get_node_by_id(ns, node.input.rhs)); */
            /* draw_parent_line(vg, &node, nodestore_get_node_by_id(ns, node.input.lhs)); */
        } break;
        }

        // To start, we probably need to get these drawing at the bounding box.
        nvgFillColor(gui->vg, nvgRGB(255,153,0));
        debug_square(gui->vg, body.draw_position.x, body.draw_position.y);
        
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

    // debug draw output bounding boxes.
    for (int i = 0; i < sb_count(outputs); i++) {
        NVGcolor color = nvgRGBf(1.0, 1.0, 0.0);
        debug_draw_bb(gui->vg, color, outputs[i].bb);
    }

    // debug draw drag target.
    if (gui->dragging_state == GUI_DRAGGING_INPUT ||
        gui->dragging_state == GUI_DRAGGING_OUTPUT) {
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
