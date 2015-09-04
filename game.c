/*
  TODO: Stuff I think I should do.
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
 */

#include <math.h>
#include "gameguy.h"
#include "game.h"

static NVGcontext* gg_Debug_vg = NULL;

/* Util */

#define CLAMP(x,xmin,xmax)  ((x) < (xmin) ? (xmin) : (x) > (xmax) ? (xmax) : (x))
#define ARRAY_COUNT(x) (sizeof(x) / sizeof(x[0]))

const char*
bool_string(bool b)
{
    return b ? "true" : "false";
}

/* Rendering */

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

int X_PADDING = 12.0;
int Y_PADDING = 12.0;

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

/* Vectors */

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

/* Bounding Box */

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

/* Nodes and Nodestore */

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
    for (int i = 0; i < ns->count; i++) {
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
    } break;
        
    case PREDICATE: {
        switch(node->predicate) {
        case LT:   txt = "<";  break;
        case GT:   txt = ">";  break;
        case LEQT: txt = "<="; break;
        case GEQT: txt = ">="; break;
        case EQ:   txt = "=="; break;
        case NEQ:  txt = "<>"; break;
        }
    } break;

    case GATE: {
        switch(node->gate) {
        case AND:
            txt = "AND";
            break;
        case OR:
            txt = "OR";
            break;
        case NOT:
            txt = "NOT";
            break;
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

/* Ship */

// @TODO: Make this just return a velocity and handle the real moving with the collision detection
// code.
void
ship_move(Entity* ship, float dt)
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

    /* ship->position.x += abs_force.x*speed*dt; */
    /* ship->position.y += abs_force.y*speed*dt; */

    // Not sure exactly how to do this yet.
    ship->velocity.x = abs_force.x*speed;
    ship->velocity.y = abs_force.y*speed;
    
    int new_rot = ship->rotation + rotation;
    if (new_rot < 0) new_rot += 360;
    ship->rotation = new_rot % 360;
}


int
ship_get_signal(const Entity* ship, Signal signal)
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
node_eval_sub_node(const Node* node, const Entity* ship)
{
    if (node == NULL) {
        return 0;
    }
    if (node->type == SIGNAL) {
        return ship_get_signal(ship, node->signal);
    } else { // Constant
        return node->constant;
    }
}


bool
node_eval(const Node* node, const NodeStore* ns, const Entity* ship)
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
nodestore_eval_thrusters(const NodeStore* ns, const Entity* ship)
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

/* Entities */

void
entity_add_flags(Entity* entity, uint32_t flags)
{
    entity->flags |= flags;
}

bool
entity_has_flags_set(Entity* entity, uint32_t flags)
{
    return entity->flags & flags;
}

/* Gamestate */

Entity*
push_entity(GameState* gamestate)
{
    Entity* new_entity;
    if (gamestate->first_free_entity) {
        new_entity = gamestate->first_free_entity;
        gamestate->first_free_entity = new_entity->next_entity;
    } else {
        assert(gamestate->num_entities < ARRAY_COUNT(gamestate->entities));
        new_entity = gamestate->entities + gamestate->num_entities++; 
    }

    // @TODO: Maybe clear the entity out.
    return new_entity;
}

/* GUI stuff */

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
nb_is_dragging(GUIState* gui, NodeBounds* nodebounds, size_t num_nodebounds, GUI_State next_gui_state)
{
    for (int i=0; i<num_nodebounds; i++) {
        if (bb_contains(nodebounds[i].bb,
                        gui->input.mouse_x - gui->input.mouse_xrel,
                        gui->input.mouse_y - gui->input.mouse_yrel)) {
            gui->state = next_gui_state;
            gui->drag_target.from_id = nodebounds[i].node->id;
            gui->drag_target.from_input_num = nodebounds[i].input_index;

            V2 nb_center = bb_center(nodebounds[i].bb);
            gui->drag_target.from_position = nb_center;
            gui->drag_target.position = nodebounds[i].bb.top_left;

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
    size_t num_bodies = 0;
    NodeBounds bodies[256] = {};
    size_t num_inputs = 0;
    NodeBounds inputs[256] = {};
    size_t num_outputs = 0;
    NodeBounds outputs[256] = {};
    size_t num_sliders = 0;
    NodeBounds sliders[256] = {};

    // Collect collision data.
    for (int i=0; i<ns->count; i++) {
        if (ns->nodes[i].id == 0) continue;
        
        // Node body bounds
        NodeBounds body;
        body.node = &ns->nodes[i];
        body.bb = node_calc_bounding_box(gui->vg, &ns->nodes[i], ns);
        body.draw_position = ns->nodes[i].position;
        bodies[num_bodies++] = body;

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
        if (ns->nodes[i].type != SIGNAL && ns->nodes[i].type != CONSTANT) {
            NodeBounds input = {};
            float centerx = ((body.bb.bottom_right.x - body.bb.top_left.x) / 2)
                + body.bb.top_left.x;
            float centery = body.bb.top_left.y;

            if (ns->nodes[i].type == PREDICATE ||
                (ns->nodes[i].type == GATE &&
                 ns->nodes[i].gate != NOT)) {
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
                // 1 of them for everything else.
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

        // Slider bounds
        if (ns->nodes[i].type == CONSTANT) {
            float height = body.bb.bottom_right.y - body.bb.top_left.y;
            float slider_basey = body.bb.bottom_right.y + height * 0.25;
            float slider_basex = body.bb.bottom_right.x + 5.0;
            float slider_length = height * 1.5;

            // @TODO: Don't just have the slider go from 0 to 700 because that's the max val
            // needed. Have the slider range be dependent on the nodes depending on it.
            // position is from the bottom.
            int current_value = ns->nodes[i].constant;
            float slider_percent = (float)current_value / 700;
            float slider_position = slider_length * slider_percent;

            V2 slider_node_position = v2(slider_basex,
                                         slider_basey - slider_position);

            BoundingBox slider_bb = boundingBox(v2_minus(slider_node_position, v2(4, 2)), 12, 8);
            NodeBounds slider = {};
            slider.node = &ns->nodes[i];
            slider.bb = slider_bb;
            slider.draw_position = slider.bb.top_left;
            sliders[num_sliders++] = slider;
        }
    }

    // Handle current gui state.
    if (gui->state == GUI_NAH) {
        if (gui->input.start_dragging) {
            // Find what we are dragging.
            bool found = nb_is_dragging(gui, sliders, num_sliders, GUI_DRAGGING_SLIDER);
            
            if (!found) {
                found = nb_is_dragging(gui, outputs, num_outputs, GUI_DRAGGING_OUTPUT);
            }

            if (!found) {
                found = nb_is_dragging(gui, inputs, num_inputs, GUI_DRAGGING_INPUT);
            }

            if (!found) {
                found = nb_is_dragging(gui, bodies, num_bodies, GUI_DRAGGING_NODE);
                if (found) {
                    // Update the node's position (why were we doing this?)
                    /* Node* node = nodestore_get_node_by_id(ns, gui->drag_target.from_id); */
                    /* node->position = gui->drag_target.position; */
                }
            }
        }

    } else if (gui->state == GUI_DRAGGING_NODE) {
        if (gui->input.end_dragging) {
            gui->state = GUI_NAH;

        } else {
            if (gui->input.mouse_motion) {
                // Continue dragging the node.
                // @TODO: return move event don't mutate
                Node* drag_node = nodestore_get_node_by_id(ns, gui->drag_target.from_id);
                drag_node->position.x += gui->input.mouse_xrel;
                drag_node->position.y += gui->input.mouse_yrel;

                // Update draw position, could probably combine this with above and
                // only loop once.
                for (int i=0; i<num_bodies; i++) {
                    if (bodies[i].node->id == drag_node->id) {
                        bodies[i].draw_position.x += gui->input.mouse_xrel;
                        bodies[i].draw_position.y += gui->input.mouse_yrel;
                        break;
                    }
                }
            }
        }

    } else if (gui->state == GUI_DRAGGING_INPUT ||
               gui->state == GUI_DRAGGING_OUTPUT) {
        if (gui->input.end_dragging) {
            // @TODO: Handle connecting the nodes here.

            // Connect input
            // @TODO: Check outputs too.
            // @TODO: Deal with what kind of nodes can connect to what kind of nodes.
            if (gui->state == GUI_DRAGGING_INPUT) {
                for (int i = 0; i < num_bodies/* num_bodies */; i++) {
                    // check if over a node.
                    if (bodies[i].node->id != gui->drag_target.from_id &&
                        bb_contains(bodies[i].bb,
                                    gui->input.mouse_x,
                                    gui->input.mouse_y) &&
                        (bodies[i].node->type == PREDICATE ||
                         bodies[i].node->type == GATE ||
                         bodies[i].node->type == CONSTANT ||
                         bodies[i].node->type == SIGNAL)
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

            gui->state = GUI_NAH;

        } else {
            if (gui->input.mouse_motion) {
                gui->drag_target.position.x = gui->input.mouse_x;
                gui->drag_target.position.y = gui->input.mouse_y;
            }
        }
    } else if (gui->state == GUI_DRAGGING_SLIDER) {
        if (gui->input.end_dragging) {
            gui->state = GUI_NAH;
            
        } else {
            if (gui->input.mouse_motion) {
                Node* constant_node = nodestore_get_node_by_id(ns, gui->drag_target.from_id);

                // @NOTE: Grab the body (this is done a few places, maybe in bodies we need
                // support for grabbing a body by id.
                for (int i=0; i<num_bodies; i++) {
                    if (bodies[i].node->id == constant_node->id) {

                        // @TODO: There's a problem in that I don't get full resolution in the
                        // slider because the slider isn't 700 pixels long. Need a way to
                        // finetune the value to any number.
                        NodeBounds body = bodies[i];

                        float height = body.bb.bottom_right.y - body.bb.top_left.y;
                        float slider_basey = body.bb.bottom_right.y + height * 0.25;
                        float slider_length = height * 1.5;

                        float new_slider_y = gui->input.mouse_y;
                        float new_slider_height = CLAMP(slider_basey - new_slider_y,
                                                        0.0,
                                                        slider_length);
                        float new_slider_percent = new_slider_height / slider_length;

                        float new_slider_value = new_slider_percent * 700.0;
                        constant_node->constant = new_slider_value;
                        break;
                    }
                }
                
            }
        }
    }

    // Draw gui elements.
    for (int i=0; i<num_bodies; i++) {
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

            if (gui_button(*gui, body.bb.bottom_right.x - 10, body.bb.top_left.y + 20, 10, 10)) {
                bodies[i].node->thruster = thruster_next(node.thruster);
            }
        } break;

        case PREDICATE: {
            node_get_text(&node, buf, 256, NULL, NULL);
            draw_text_box(gui->vg, buf, body.draw_position.x, body.draw_position.y);

            V2 center = bb_center(body.bb);
            if (gui_button(*gui, center.x-2.5, body.bb.top_left.y+25, 10, 10)) {
                bodies[i].node->predicate = predicate_next(node.predicate);
            }
            
        } break;

        case CONSTANT: {
            node_get_text(&node, buf, 256, NULL, NULL);
            draw_text_box(gui->vg, buf, body.draw_position.x, body.draw_position.y);

            // @TODO: Handle making the value change faster based on how long it has been
            //        held down.
            V2 center = bb_center(body.bb);
            if (gui_button(*gui, center.x+10, center.y-10, 5, 5)) {
                // Up
                body.node->constant++;
            }

            if (gui_button(*gui, center.x+10, center.y+10, 5, 5)) {
                // Down
                body.node->constant--;
            }
        } break;

        case SIGNAL: {
            node_get_text(&node, buf, 256, NULL, NULL);
            draw_text_box(gui->vg, buf, body.draw_position.x, body.draw_position.y);

            if (gui_button(*gui, body.bb.bottom_right.x - 10, body.bb.top_left.y + 20, 10, 10)) {
                bodies[i].node->signal = signal_next(node.signal);
            }
        } break;

        case GATE: {
            node_get_text(&node, buf, 256, NULL, NULL);
            draw_text_box(gui->vg, buf, body.draw_position.x, body.draw_position.y);

            if (gui_button(*gui, body.bb.bottom_right.x - 10, body.bb.top_left.y + 20, 10, 10)) {
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
    debug_draw_nodebounds(gui->vg, inputs, num_inputs);
    debug_draw_nodebounds(gui->vg, outputs, num_outputs);
    debug_draw_nodebounds(gui->vg, sliders, num_sliders);

    // debug draw drag target.
    if (gui->state == GUI_DRAGGING_INPUT ||
        gui->state == GUI_DRAGGING_OUTPUT) {
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

void
setup_level(GameState* state) {
    // @NOTE: zero entities out.
    memset(state->entities, 0, sizeof(Entity) * ARRAY_COUNT(state->entities));
    state->num_entities = 0;
    state->first_free_entity = NULL;

    Entity* ship = push_entity(state);
    ship->type = EntityType_SHIP;
    ship->position = v2(300, 99);
    ship->rotation = 0;

    entity_add_flags(ship, EntityFlag_COLLIDES);

    CollisionRect* s1 = &ship->collision_pieces[ship->num_collision_pieces++];
    s1->offset = v2(-10, -15);
    s1->size = v2(20, 40);
    CollisionRect* s2 = &ship->collision_pieces[ship->num_collision_pieces++];
    s2->offset = v2(-20, -25);
    s2->size = v2(15, 30);
    CollisionRect* s3 = &ship->collision_pieces[ship->num_collision_pieces++];
    s3->offset = v2(5, -25);
    s3->size = v2(15, 30);

    Entity* goal = push_entity(state);
    goal->type = EntityType_GOAL;
    goal->position = v2(500, 600);
    goal->rotation = 0;

    entity_add_flags(goal, EntityFlag_COLLIDES);

    CollisionRect* g = &goal->collision_pieces[goal->num_collision_pieces++];
    g->offset = v2(-10, -10);
    g->size = v2(20, 20);

    state->current_level = 1;

    Entity* test_block = push_entity(state);
    test_block->type = EntityType_BOUNDRY;
    test_block->position = v2(300, 300);
    test_block->rotation = 0;

    entity_add_flags(test_block, EntityFlag_COLLIDES);

    CollisionRect* r = &test_block->collision_pieces[test_block->num_collision_pieces++];
    r->offset = v2(30, 0);
    r->size = v2(50, 50);

    state->status = RUNNING;
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

    setup_level(state);
    /* state->collision_area_x = state->collision_area_y = state->collision_area_width = state->collision_area_height = 0; */

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

    // @TODO: All kinds of gates!
    if (gui_button(state->gui, 10, 10, 50, 25)) {
        nodestore_add_signal(&state->node_store, 10, 45, POS_X);
        log_info("adding signal node");
    }

    if (gui_button(state->gui, 70, 10, 50, 25)) {
        nodestore_add_constant(&state->node_store, 10, 45, 0);
        log_info("adding constant node");
    }

    if (gui_button(state->gui, 130, 10, 50, 25)) {
        nodestore_add_predicate(&state->node_store, 10, 45, EQ);
        log_info("adding predicate node");
    }

    if (gui_button(state->gui, 190, 10, 50, 25)) {
        nodestore_add_gate(&state->node_store, 10, 45, AND);
        log_info("adding gate node");
    }

    if (gui_button(state->gui, 250, 10, 50, 25)) {
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

    char* reset = "Reset";
    if (gui_button_with_text(state->gui, 660, 2.5, 10, 5, reset)) {
        setup_level(state);
    }

    if (state->status == RUNNING) {
        // Update Entities
        for (int i = 0; i < state->num_entities; i++) {
            Entity* entity = state->entities + i;
            switch(entity->type) {

            case (EntityType_NAH): {
                // Nah
            } break;

            case (EntityType_SHIP): {
                // Update Ship

                // @TODO: Sort thrusters before
                Thrusters new_thrusters = nodestore_eval_thrusters(&state->node_store,
                                                                   entity);
                entity->thrusters = new_thrusters;
                ship_move(entity, dt);


                // @HARDCODE: Fix with collision detection.
                // Goal is at (500, 600)
                if (bounds_contains(500 - 10,
                                    600 - 10,
                                    500 + 10,
                                    600 + 10,
                                    entity->position.x, entity->position.y)) {
                    // Won the level!
                    state->status = WON;
                }

                // @TODO: pull this out, this is hella useful.
                // debug print
                char buf2[64] = {'\0'};

                snprintf(buf2, 64, "position: (%f, %f), rotation: %d",
                         entity->position.x,
                         entity->position.y,
                         entity->rotation);

                debug_draw_text(vg, 10, SCREEN_HEIGHT - 50, 24, buf2);

            } break;

            case (EntityType_BOUNDRY): {
                
            } break;

            case (EntityType_GOAL): {

            } break;
            }
        }
        // @NOTE: This is my first pass at collision detection so I'm guessing it won't be that good.
        // One thing you can do is wrap complex objects in a AABB bounding box, do the collision detect
        // on that and only if that passes do the check on their actual geometry. Speeds up the scene
        // a lot probably.

        // Move entities based on speed. Handle collisions.
        // @OPTOMIZE:
        // Start by checking each entity against each other entity? Seems like a bit much.
        // Also feel like looping over all entities 3 times is a bad move. Re-evaluate later.
 
        // @TODO: Add rotations, starting with simpler case of just AABB.
        for (int i = 0; i < state->num_entities; i++) {
            Entity* entity = state->entities + i;
            if (entity->type == EntityType_NAH) continue;

            // @DEBUG
            if (entity->type == EntityType_BOUNDRY) continue;

            // @NOTE: This moves entities one at a time. Could miss collisions.
            // Not a big deal if only the rocket moves but problematic if two moving objects.
            V2 next_entity_position = v2_plus(entity->position, v2_scale(entity->velocity, dt));
            bool collides = false;

            for (int j = 0; j < state->num_entities; j++) {
                Entity* collision_entity = state->entities + j;
                if (collision_entity->type == EntityType_NAH) continue;
                if (collision_entity == entity) continue; // Don't collide with self.

                // Check if any of the entity collision pieces overlap any other ones.
                for (int ep = 0; ep < entity->num_collision_pieces; ep++) {
                    CollisionRect entity_piece = entity->collision_pieces[ep];

                    for (int cep = 0; cep < collision_entity->num_collision_pieces; cep++) {
                        CollisionRect collision_entity_piece = collision_entity->collision_pieces[cep];

                        // Check if they collide.
                        float area_width = collision_entity_piece.width + entity_piece.width;
                        float area_height = collision_entity_piece.height + entity_piece.height;

                        V2 bottom_left = v2_plus(collision_entity->position,
                                              collision_entity_piece.offset);

                        // Translate to entity_piece origin
                        bottom_left = v2_minus(bottom_left, entity_piece.size);

                        // Translate to entity origin
                        bottom_left = v2_minus(bottom_left, entity_piece.offset);

                        /* // @TODO: Have a way better way to debug this. */
                        /* state->collision_area_x = bottom_left.x; */
                        /* state->collision_area_y = bottom_left.y; */
                        /* state->collision_area_width = area_width; */
                        /* state->collision_area_height = area_height; */
                    
                        if ((next_entity_position.x > bottom_left.x &&
                             next_entity_position.x < bottom_left.x + area_height) &&
                            (next_entity_position.y > bottom_left.y &&
                             next_entity_position.y < bottom_left.y + area_width)) {
                            collides = true;

                            /* log_info("me: (%f.2,%f.2) piece: (%f.2,%f.2) w:%f.2, h:%f.2", */
                            /*          next_entity_position.x, next_entity_position.y, */
                            /*          bottom_left.x, bottom_left.y, area_width, area_height); */
                        
                            break;
                        }
                    }
                    if (collides) break;
                }
                if (collides) break;
            }

            if (collides) {
                // Dead
                state->status = DIED;
            } else {
                entity->position = next_entity_position;
            }
        }
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

        // Render Entities
        for (int i = 0; i < state->num_entities; i++) {
            Entity* entity = state->entities + i;

            switch(entity->type) {

            case (EntityType_NAH): {
                // Nah
            } break;

            case (EntityType_SHIP): {
                // Draw Ship
                nvgSave(vg);
                {
                    nvgTranslate(vg,
                                 entity->position.x,
                                 -entity->position.y);
                    nvgRotate(vg, -deg_to_rad(entity->rotation));
                    draw_ship(vg,
                              entity->thrusters,
                              false);
                }
                nvgRestore(vg);
                
            } break;

            case (EntityType_BOUNDRY): {

            } break;

            case (EntityType_GOAL): {
                // Draw the goal
                nvgBeginPath(vg);
                nvgRect(vg, entity->position.x-10, -entity->position.y-10, 20, 20);
                nvgFillColor(vg, nvgRGBf(1.0,1.0,0));
                nvgFill(vg);
            } break;
            }

            // Render collision geometry for debugging.
            for (int c=0; c<entity->num_collision_pieces; c++) {
                CollisionRect rect = entity->collision_pieces[c];
                
                nvgBeginPath(vg);
                nvgRect(vg,
                        entity->position.x + rect.x,
                        -entity->position.y - rect.y - rect.height,
                        rect.width,
                        rect.height);
                nvgStrokeColor(vg, nvgRGBf(1, 1, 1));
                nvgStroke(vg);
            }
        }

        /* // Debug draw collision Area. This only works for 1.... */
        /* nvgBeginPath(vg); */
        /* nvgRect(vg, */
        /*         state->collision_area_x, */
        /*         -state->collision_area_y - state->collision_area_height, */
        /*         state->collision_area_width, */
        /*         state->collision_area_height); */
        /* nvgStrokeColor(vg, nvgRGBf(1, 0, 0)); */
        /* nvgStroke(vg); */
        
    }
    nvgRestore(vg);
}

const gg_Game gg_game_api = {
    .gamestate_size = sizeof(GameState),
    .init = game_setup,
    .update_and_render = game_update_and_render
};
