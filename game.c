// @TODO: @IMPORTANT: JUST DO IT
// think next could be more levels? collision detection?

// Some stuff I want to do.
// Break the nodes into more pieces, math nodes.
// Figure out how to make the UI not suck.
// Sort nodes topolocically so eval is *fast*.
// Work on some more levels.

#include <math.h>
#include "gameguy.h"
#include "game.h"

// this is strait up taken from stb.h which I couldn't figure out how to include
#define clamp(x,xmin,xmax)  ((x) < (xmin) ? (xmin) : (x) > (xmax) ? (xmax) : (x))
#define ARRAY_COUNT(x) (sizeof(x) / sizeof(x[0]))

static const int X_PADDING = 12.0;
static const int Y_PADDING = 12.0;

const char*
bool_string(bool b)
{
    return b ? "true" : "false";
}


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


V2
v2_rotate(const V2 v, const float radians)
{
    float nx = v.x * cos(radians) - v.y * sin(radians);
    float ny = v.x * sin(radians) + v.y * cos(radians);
    return v2(nx, ny);
}


BoundingBox
boundingBox(V2 top_left, float width, float height)
{
    return (BoundingBox){top_left, v2_plus(top_left, v2(width, height))};
}


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

// So it occurs to be that I can do this in a better way.
// I can sort the nodes topologically so that all parents will be found
// earlier in the array than their children. Then evaluation of all the nodes
// can be done with one linear scan down the array that keeps track of the results.

// This way I don't have to do the recursive evaluation function and I can easily use
// the state of each node in visualizations.

Node*
nodestore_get_node_by_id(const NodeStore* ns, int id)
{
    if (id == 0) return NULL;

    assert(id > 0);
    assert(id <= ns->last_id);
    
    // @TODO: Better hash function.
    uint32_t hash_bucket = id & (ARRAY_COUNT(ns->id_hash) - 1);
    for (Node* node = ns->id_hash[hash_bucket];
         node;
         node = node->next_in_hash) {
        if (node->id == id) {
            return node;
        }
    }
    
    return NULL;
}


Node*
nodestore_init_new_node(NodeStore* ns, float pos_x, float pos_y)
{
    // Add to store.
    // @NOTE: id == 0 means not initialized. Used when traversing.
    int id = ++ns->last_id;

    Node* node;
    if (ns->first_free_node) {
        node = ns->first_free_node;
        ns->first_free_node = node->next_in_hash;
    } else {
        node = &(ns->nodes[ns->count++]);
    }

    // Add to hash.
    // @TODO: Better hash function.
    uint32_t hash_bucket = id & (ARRAY_COUNT(ns->id_hash) - 1);
    node->next_in_hash = ns->id_hash[hash_bucket];
    ns->id_hash[hash_bucket] = node;

    node->id = id;
    node->position.x = pos_x;
    node->position.y = pos_y;

    node->input.lhs = 0;
    node->input.rhs = 0;

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
nodestore_destory_node(NodeStore* ns, int id)
{
    assert(id <= ns->last_id);
    assert(id > 0);

    // @TODO: Better hash function.
    uint32_t hash_bucket = id & (ARRAY_COUNT(ns->id_hash) - 1);
    for (Node** node = &ns->id_hash[hash_bucket];
         *node;
         node = &(*node)->next_in_hash) {
        if ((*node)->id == id) {
            Node* removed_node = *node;
            *node = (*node)->next_in_hash;
            removed_node->next_in_hash = ns->first_free_node;
            removed_node->id = 0;
            ns->first_free_node = removed_node;
            break;
        }
    }

    // Fix all pointers, if a pointer pointed to this node set it to 0;
    for(int i = 0; i < ns->count; i++) {
        if (ns->nodes[i].input.lhs == id) {
            ns->nodes[i].input.lhs = 0;
        }
        if (ns->nodes[i].input.rhs == id) {
            ns->nodes[i].input.rhs = 0;
        }
    }
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

void
debug_draw_square(NVGcontext* vg, float x, float y)
{
    nvgBeginPath(vg);
    nvgRect(vg, x, y, 1, 1);
    nvgFill(vg);
}

void
debug_draw_text(NVGcontext* vg, float x, float y, int size, const char* txt)
{
    nvgSave(vg);
    nvgFontSize(vg, size);
    nvgFillColor(vg, nvgRGBf(1, 1, 1));
    nvgText(vg, x, y, txt, NULL);
    nvgRestore(vg);
}

void
debug_draw_nodebounds(NVGcontext* vg, NodeBounds* nb, size_t num_nb)
{
    for (int i = 0; i < num_nb; i++) {
        NVGcolor color = nvgRGBf(0.0, 1.0, 0.0);
        debug_draw_bb(vg, color, nb[i].bb);
    }

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


// draws a button there, if there was a click of it this frame returns true
// @TODO: Text
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


bool
nb_is_dragging(GUIState* gui, NodeBounds* nodebounds, size_t num_nodebounds, DraggingState next_dragging_state)
{
    for (int i=0; i<num_nodebounds; i++) {
        if (bb_contains(nodebounds[i].bb,
                         gui->input.mouse_x - gui->input.mouse_xrel,
                         gui->input.mouse_y - gui->input.mouse_yrel)) {
            gui->dragging_state = next_dragging_state;
            gui->drag_target.from_id = nodebounds[i].node->id;
            gui->drag_target.from_input_num = nodebounds[i].input_index;

            V2 nb_center = bb_center(nodebounds[i].bb);
            gui->drag_target.from_position = nb_center;
            gui->drag_target.position = nodebounds[i].bb.top_left;


            if (gui->input.mouse_motion) {
                gui->drag_target.position.x += gui->input.mouse_xrel;
                gui->drag_target.position.y += gui->input.mouse_yrel;
            }

            return true;
        }
    }

    return false;
}


// @TODO: This really could just be inline int the update function. There's no real reason to have
// it pulled out like this.
NodeEvent
gui_nodes(GUIState* gui, NodeStore* ns)
{
    //@TODO: Replace these with variable length arrays, you do not need to allocate.
    // They also could just be parallel arrays in the gamestate and then I don't have to
    // calculate them every single frame....
    size_t num_bodies = 0;
    NodeBounds bodies[256] = {};
    size_t num_inputs = 0;
    NodeBounds inputs[256] = {};
    size_t num_outputs = 0;
    NodeBounds outputs[256] = {};
    size_t num_constants = 0;
    NodeBounds constants[256] = {};

    // Collect collision data.
    for (int i=0; i<ns->count; i++) {
        if (ns->nodes[i].id == 0) continue;
        if (ns->nodes[i].type != SIGNAL && ns->nodes[i].type != CONSTANT) {
            // Node body bounds
            NodeBounds body;
            body.node = &ns->nodes[i];
            body.bb = node_calc_bounding_box(gui->vg, &ns->nodes[i], ns);
            body.draw_position = ns->nodes[i].position;
            bodies[num_bodies++] = body;

           // Constant bounds
            Node* lhs = nodestore_get_node_by_id(ns, ns->nodes[i].input.lhs);
            Node* rhs = nodestore_get_node_by_id(ns, ns->nodes[i].input.rhs);

            if (NULL != lhs && lhs->type == CONSTANT) {
                NodeBounds constant = {};
                constant.node = lhs;
                constant.draw_position = v2(body.draw_position.x+7,
                                            body.draw_position.y+25);
                constant.bb = (BoundingBox){constant.draw_position,
                                            v2_plus(constant.draw_position, v2(10, 10))};
                constants[num_constants++] = constant;
            }

            if (NULL != rhs && rhs->type == CONSTANT) {
                NodeBounds constant = {};
                constant.node = rhs;
                constant.draw_position = (V2){.x = body.bb.bottom_right.x-12,
                                              .y = body.bb.top_left.y+25};
                constant.bb = (BoundingBox){constant.draw_position,
                                            v2_plus(constant.draw_position, v2(10, 10))};
                constants[num_constants++] = constant;
            }

            // Output bounds
            if (ns->nodes[i].type != THRUSTER) {
                NodeBounds output = {};
                output.node = &ns->nodes[i];
                float centerx = ((body.bb.bottom_right.x - body.bb.top_left.x) / 2)
                    + body.bb.top_left.x;
                float centery = body.bb.bottom_right.y;
                // @HARDCODE
                output.bb = (BoundingBox){v2(centerx - 7.5, centery - 5),
                                          v2(centerx + 7.5, centery + 5)};
                output.draw_position = output.bb.top_left;
                outputs[num_outputs++] = output;
            }

            // Inputs bounds
            if (ns->nodes[i].type != PREDICATE) {
                NodeBounds input = {};
                float centerx = ((body.bb.bottom_right.x - body.bb.top_left.x) / 2)
                    + body.bb.top_left.x;
                float centery = body.bb.top_left.y;

                if (ns->nodes[i].type == GATE &&
                    ns->nodes[i].gate != NOT) {
                    // 2 inputs for AND and OR gates.
                    input.node = &ns->nodes[i];

                    input.bb = (BoundingBox){v2(body.bb.top_left.x + 2.5, centery - 5),
                                             v2(body.bb.top_left.x + 17.5, centery + 5)};
                    input.draw_position = input.bb.top_left;
                    input.input_index = 1;
                    if (input.node->input.lhs != 0) {
                        input.input_to = nodestore_get_node_by_id(ns, input.node->input.lhs);
                    }
                    inputs[num_inputs++] = input;

                    input.bb = (BoundingBox){v2(body.bb.bottom_right.x - 17.5, centery - 5),
                                             v2(body.bb.bottom_right.x - 2.5, centery + 5)};
                    input.draw_position = input.bb.top_left;
                    input.input_index = 2;
                    if (input.node->input.rhs != 0) {
                        input.input_to = nodestore_get_node_by_id(ns, input.node->input.rhs);
                    } else {
                        input.input_to = NULL;
                    }
                    inputs[num_inputs++] = input;
                } else {
                    // 1 of them for NOT gates and Thrusters.
                    input.node = &ns->nodes[i];
                    input.bb = (BoundingBox){v2(centerx - 7.5, centery - 5),
                                             v2(centerx + 7.5, centery + 5)};
                    input.draw_position = input.bb.top_left;
                    input.input_index = 1;
                    if (input.node->parent != 0) {
                        input.input_to = nodestore_get_node_by_id(ns, input.node->parent);
                    }
                    inputs[num_inputs++] = input;
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
            // Find what we are dragging.
            bool found = nb_is_dragging(gui, outputs, num_outputs, GUI_DRAGGING_OUTPUT);

            if (!found) {
                found = nb_is_dragging(gui, inputs, num_inputs, GUI_DRAGGING_INPUT);
            }

            if (!found) {
                found = nb_is_dragging(gui, constants, num_constants, GUI_DRAGGING_CONSTANT);
            }

            if (!found) {
                found = nb_is_dragging(gui, bodies, num_bodies, GUI_DRAGGING_NODE);
                if (found) {
                    // Update the node's position
                    Node* node = nodestore_get_node_by_id(ns, gui->drag_target.from_id);
                    node->position = gui->drag_target.position;
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
                for (int i=0; i<num_bodies/* num_bodies */; i++) {
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
                for (int i = 0; i < num_bodies/* num_bodies */; i++) {
                    // check if over a node.
                    if (bodies[i].node->id != gui->drag_target.from_id &&
                        bb_contains(bodies[i].bb,
                                    gui->input.mouse_x,
                                    gui->input.mouse_y) &&
                        (bodies[i].node->type == PREDICATE ||
                         bodies[i].node->type == GATE)
                        ) {
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
            V2 diff = v2_minus(gui->drag_target.position, gui->drag_target.from_position);
            float angle = atan2(-diff.y, diff.x) + 3.14159/4.0;   // angle is between 0 and pi/2
            float scale_value = (clamp(angle, 0, 3.14159/2.0) / 3.14159/2.0) / 0.25;

            // To get the right value I need to know what the signal we are comparing it to is....
            // @TODO: need to know what signal I am comparing this to.
            float value = (int)(scale_value * 700);
            gui->drag_target.value = value;

            // Debug draw dragging line.
            nvgSave(gui->vg);
            nvgBeginPath(gui->vg);
            nvgMoveTo(gui->vg, gui->drag_target.from_position.x, gui->drag_target.from_position.y);
            nvgLineTo(gui->vg, gui->drag_target.position.x, gui->drag_target.position.y);
            nvgStrokeColor(gui->vg, nvgRGBf(0.3, 0.3, 1.0));
            nvgStroke(gui->vg);
            nvgRestore(gui->vg);

            // Debug draw angle @TODO: a gameguy debug text drawing thing would be helpful.
            char buf[64] = {'\0'};
            snprintf(buf, 64, "angle: %f, scale_value: %f, value: %f", angle, scale_value, value);
            debug_draw_text(gui->vg, gui->drag_target.from_position.x,
                            gui->drag_target.from_position.y - 35, 14, buf);
        }
    }

    // Draw gui elements.
    for (int i=0; i<num_bodies/* num_bodies */; i++) {
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
                // Constant.
                // @TODO: Draw something for editing other than the current debug stuff drawn above.
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
                // Constant.
                // @TODO: Draw something for editing other than the current debug stuff drawn above.
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
        debug_draw_square(gui->vg, body.draw_position.x, body.draw_position.y);
    }

    // Draw connection lines. Arows from output to input?
    for (int i = 0; i < num_inputs; i++) {
        if (inputs[i].input_to != NULL) {
            // draw line to input_to's output.
            nvgSave(gui->vg);
            nvgBeginPath(gui->vg);

            V2 input_center = bb_center(inputs[i].bb);
            nvgMoveTo(gui->vg, input_center.x, inputs[i].bb.top_left.y);
            // @TODO: Bezier curves instead of strait lines

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
                    inputs[i].node->input.lhs = 0;
                } else {
                    inputs[i].node->input.rhs = 0;
                }
            }
        }
    }

    // Draw destroy buttons for nodes.
    for (int i = 0; i < num_bodies; i++) {
        if (gui_button(*gui,
                   bodies[i].bb.top_left.x, bodies[i].bb.top_left.y,
                       10, 10)) {
            nodestore_destory_node(ns, bodies[i].node->id);
        }
    }

    // Debug drawing
    debug_draw_nodebounds(gui->vg, bodies, num_bodies);
    debug_draw_nodebounds(gui->vg, constants, num_constants);
    debug_draw_nodebounds(gui->vg, inputs, num_inputs);
    debug_draw_nodebounds(gui->vg, outputs, num_outputs);

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

    return (NodeEvent){.type = NE_NAH};
}


static NVGcontext* gg_Debug_vg = NULL;

// Rendering Code
void
draw_parent_line(NVGcontext* vg, const Node* node, const Node* parent)
{
    if (parent) {
        nvgSave(vg);
        {
            nvgStrokeColor(vg, nvgRGBf(200.0, 200.0, 200.0));
            nvgBeginPath(vg);
            nvgTextLineHeight(vg, 100);
            nvgMoveTo(vg, node->position.x, node->position.y);
            nvgLineTo(vg, parent->position.x, parent->position.y);
            nvgStroke(vg);
        }
        nvgRestore(vg);
    }
}

void
ship_move(Ship* ship, float dt)
{
    V2 force = v2(0.0, 0.0);
    int rotation = 0;

    if (ship->thrusters.bp) {
        force = v2_plus(force, v2(1, 0));
        rotation -= 1;
    }

    if (ship->thrusters.bs) {
        force = v2_plus(force, v2(-1, 0));
        rotation += 1;
    }

    if (ship->thrusters.sp) {
        force = v2_plus(force, v2(1, 0));
        rotation += 1;
    }

    if (ship->thrusters.ss) {
        force = v2_plus(force, v2(-1, 0));
        rotation -= 1;
    }

    if (ship->thrusters.boost) {
        force = v2_plus(force, v2(0, 5));
    }

    V2 abs_force = v2_rotate(force, deg_to_rad(ship->rotation));
    float speed = 50;

    ship->position.x += abs_force.x*speed*dt;
    ship->position.y += abs_force.y*speed*dt;
    int new_rot = ship->rotation + rotation;
    if (new_rot < 0) new_rot += 360;
    ship->rotation = new_rot % 360;
}


int
ship_get_signal(const Ship* ship, Signal signal)
{
    switch(signal) {
    case POS_X:
        return ship->position.x;
        break;
    case POS_Y:
        return ship->position.y;
        break;
    case ROTATION:
        return ship->rotation;
        break;
    }

    //error
    return -1;
}


int
node_eval_sub_node(const Node* node, const Ship* ship)
{
    if (node->type == SIGNAL) {
        return ship_get_signal(ship, node->signal);
    } else { // Constant
        return node->constant;
    }
}


bool
node_eval(const Node* node, const NodeStore* ns, const Ship* ship)
{
    if (NULL == node) {
        return false;
    }

    // SOME DEBUG PRINTING!
    switch(node->type) {
    case SIGNAL: {
        // only a sub node
    } break;
    case CONSTANT: {
        // only a sub node
    } break;
    case PREDICATE: {
        int lhs = node_eval_sub_node(nodestore_get_node_by_id(ns,
                                                              node->input.lhs),
                                     ship);
        int rhs = node_eval_sub_node(nodestore_get_node_by_id(ns,
                                                              node->input.rhs),
                                     ship);

        switch(node->predicate) {
        case LT:
            return lhs < rhs;
            break;
        case GT:
            return lhs > rhs;
            break;
        case LEQT:
            return lhs <= rhs;
            break;
        case GEQT:
            return lhs >= rhs;
            break;
        case EQ:
            return lhs == rhs;
            break;
        case NEQ:
            return lhs != rhs;
            break;
        }
    } break;
    case GATE: {
        bool lhs = node_eval(nodestore_get_node_by_id(ns, node->input.lhs),
                             ns, ship);
        bool rhs = false;

        if (node->gate != NOT) {
            rhs = node_eval(nodestore_get_node_by_id(ns, node->input.rhs),
                            ns, ship);
        }

        switch(node->gate) {
        case AND:
            return lhs && rhs;
            break;
        case OR:
            return lhs || rhs;
            break;
        case NOT:
            return !lhs;
            break;
        }

    } break;
    case THRUSTER: {
        return node_eval(nodestore_get_node_by_id(ns, node->parent), ns, ship);
    } break;
    }

    // This is actually an error tho...
    return false;

}


// @TODO: Store these in a topologically sorted order so I can just traverse them
//        instead of recuing through them.
Thrusters
nodestore_eval_thrusters(const NodeStore* ns, const Ship* ship)
{
    Thrusters out_thrusters = {false, false, false, false, false};

    // Iterate over thruster nodes
    for (int i = 1; i <= ns->last_id; i++) {
        Node* node = nodestore_get_node_by_id(ns, i);
        if (!node) continue;
        bool value = node_eval(node, ns, ship);

        // @TODO: IF DEBUG
        nvgSave(gg_Debug_vg);
        {
            if (value) {
                nvgFillColor(gg_Debug_vg, nvgRGBf(0,1,0));
            } else {
                nvgFillColor(gg_Debug_vg, nvgRGBf(1,0,0));
            }
            debug_draw_square(gg_Debug_vg, node->position.x-25, node->position.y);
        }
        nvgRestore(gg_Debug_vg);

        if (node->type == THRUSTER) {
            switch(node->thruster) {
            case BP:
                out_thrusters.bp |= value;
                break;
            case BS:
                out_thrusters.bs |= value;
                break;
            case SP:
                out_thrusters.sp |= value;
                break;
            case SS:
                out_thrusters.ss |= value;
                break;
            case BOOST:
                out_thrusters.boost |= value;
                break;
            }
        }
    }

    return out_thrusters;
}

// Really need to get farther in handmade hero and see how casey does stuff.
// Entity stores in tables, sparse storage, stuff like that.
void
gamestate_load_level_one(GameState* state) {
    state->player_ship.position.x = 300;
    state->player_ship.position.y = 99;
    state->player_ship.rotation = 0;
    state->num_obstacles = 0;

    state->goal = v2(300, 600);
    state->current_level = 1;
}

void
gamestate_load_level_two(GameState* state) {
    state->player_ship.position.x = 300;
    state->player_ship.position.y = 99;
    state->player_ship.rotation = 0;
    state->num_obstacles = 0;

    state->goal = v2(500, 600);
    state->current_level = 2;
}

void
gamestate_load_level_three(GameState* state) {
    state->player_ship.position.x = 300;
    state->player_ship.position.y = 99;
    state->player_ship.rotation = 0;

    state->obstacles[0] = boundingBox(v2(295, 595), 10, 10);
    state->obstacles[1] = boundingBox(v2(495, 95), 10, 10);
    state->num_obstacles = 2;

    state->goal = v2(500, 600);
    state->current_level = 3;
}


static void*
game_setup(void* game_state, NVGcontext* vg)
{
    log_info("Setting up game");
    GameState* state = (GameState*)game_state;

    // @TODO: If you have more than one font you need to store a
    // reference to this.
    int font = nvgCreateFont(vg,
                             "basic",
                             "SourceSansPro-Regular.ttf");
    // Assert that the font got loaded.
    assert(font >= 0);

    /* nodestore_init(&state->node_store, 5); */

    gamestate_load_level_three(state);

    return state;
}


static void
game_update_and_render(void* gamestate,
                       NVGcontext* vg,
                       gg_Input input,
                       float dt)
{
    // Setup debug frame.
    if (!gg_Debug_vg) {
        gg_Debug_vg = vg;
    }

    // Setup frame.
    GameState* state = (GameState*)gamestate;
    state->gui.vg = vg;
    state->gui.input = input;

    // Update Gui
    if (gui_button(state->gui, 10, 10, 50, 25)) {
        int a = nodestore_add_constant(&state->node_store, 0, 0, 0);
        int b = nodestore_add_signal(&state->node_store, 0, 0, POS_X);
        int c = nodestore_add_predicate(&state->node_store, 10, 45, EQ);
        Node* node_c = nodestore_get_node_by_id(&state->node_store, c);
        node_c->input.rhs = a;
        node_c->input.lhs = b;

        log_info("adding predicate node");
    }

    if (gui_button(state->gui, 70, 10, 50, 25)) {
        nodestore_add_gate(&state->node_store, 10, 45, AND);
        log_info("adding gate node");
    }

    if (gui_button(state->gui, 130, 10, 50, 25)) {
        nodestore_add_thruster(&state->node_store, 10, 45, BOOST);
        log_info("adding thruster node");
    }

    // @TODO: It is very unfortunate that rendering happens in here....
    // I can probably do this return events system if I want to. Don't know if I need to though.
    NodeEvent event = gui_nodes(&state->gui, &state->node_store);


    
    switch(event.type) {
    case NE_NAH:
        break;
    }

    // @TODO: decide if you want a pause.
    /* state->running = true; */

    // Reset button
    // @TODO: reset the whole level not just the ship.
    if (gui_button_with_text(state->gui, 660, 2.5, 10, 5, "Reset")) {
            state->player_ship.position.x = 300;
            state->player_ship.position.y = 99;
            state->player_ship.rotation = 0;
            state->status = RUNNING;
    }

    // Update rockets.
    // @TODO: Collision detection!
    // Need to see if a rocket hits anything. I can do a minkowsky thing I think
    if (state->status == RUNNING) {
        // @TODO: Sort thrusters before
        Thrusters new_thrusters = nodestore_eval_thrusters(&state->node_store,
                                                           &state->player_ship);
        state->player_ship.thrusters = new_thrusters;
        ship_move(&state->player_ship, dt);
    }

    // Collision Detection.
    // Can't use bounding boxes for this as the rocket is not always axis aligned.
    // @HARDCODE
    // what do we do, like top and bottom and leftmost and rightmost?
    
    // If rocket hits something, you explode
    // If rocket goes off the screen, you are lost in space!

    // Evaluate anything that happened.
    // If rocket is in the goal circle, then you win!
    // TODO: use the whole rocket bb instead of just the center point.
    if (bounds_contains(state->goal.x - 10,
                        state->goal.y - 10,
                        state->goal.x + 10,
                        state->goal.y + 10,
                        state->player_ship.position.x, state->player_ship.position.y)) {

        // Won the level!
        state->status = WON;
        
    }

    // Render

    // Space background!
    nvgBeginPath(vg);
    nvgRect(vg, 660, 10, 600, 700);
    nvgFillColor(vg, nvgRGBf(0.25, 0.25, 0.25));
    nvgFill(vg);

    // Level info
    char buf[64] = {'\0'};
    snprintf(buf, 64, "level %d", state->current_level);
    debug_draw_text(vg, 660, 27, 24, buf);

    nvgSave(vg);
    {
        // x,y positions will need their y flipped to be drawn in the proper
        // place. The translates take care of the rest of the movement. Math in
        // space can be done in cartesian coordinates, just need this work to
        // draw in the right place. Drawing will be done in normal nvg
        // coordinates so stuff like text works. Collision detection will have
        // to be in cartesian and seperate from rendering.
        nvgTranslate(vg, 660, 10);
        nvgTranslate(vg, 0, 700);

        // @TODO: Figure out what you have to do to not have to make all y coordinates
        // negative here.

        // draw ship
        nvgSave(vg);
        {
            nvgTranslate(vg,
                         state->player_ship.position.x,
                         -state->player_ship.position.y);
            nvgRotate(vg, -deg_to_rad(state->player_ship.rotation));
            draw_ship(vg,
                      state->player_ship.thrusters,
                      false);
        }
        nvgRestore(vg);

        // draw obsticles
        for (int i=0; i < state->num_obstacles; i++) {
            nvgBeginPath(vg);
            nvgRect(vg,
                    state->obstacles[i].top_left.x,
                    -state->obstacles[i].top_left.y,
                    state->obstacles[i].bottom_right.x-state->obstacles[i].top_left.x,
                    -state->obstacles[i].bottom_right.y+state->obstacles[i].top_left.y);
            nvgFillColor(vg, nvgRGBf(0.0, 1.0, 1.0));
            nvgFill(vg);
        }

        // draw the goal
        nvgBeginPath(vg);
        nvgRect(vg, state->goal.x-10, -state->goal.y-10, 20, 20);
        nvgFillColor(vg, nvgRGBf(1.0,1.0,0));
        nvgFill(vg);
    }
    nvgRestore(vg);

    // @TODO: pull this out, this is hella useful.
    // debug print
    char buf2[64] = {'\0'};

    snprintf(buf2, 64, "position: (%f, %f), rotation: %d",
             state->player_ship.position.x,
             state->player_ship.position.y,
             state->player_ship.rotation);

    debug_draw_text(vg, 10, SCREEN_HEIGHT - 50, 24, buf2);

    
}

const gg_Game gg_game_api = {
    .gamestate_size = sizeof(GameState),
    .init = game_setup,
    .update_and_render = game_update_and_render
};
