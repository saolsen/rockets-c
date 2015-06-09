#include "gameguy.h"
#include "game.h"

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

    /* log_info("force: %f, %f", force.x, force.y); */

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

Thrusters
nodestore_eval_thrusters(const NodeStore* ns, const Ship* ship)
{
    Thrusters out_thrusters = {false, false, false, false, false};

    // iterate over thruster nodes
    for (int i = 0; i < ns->next_id; i++) {
        Node* node = nodestore_get_node_by_id(ns, i);
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


void
nodestore_load_test_nodes(NodeStore* ns)
{
    // HOLY MOLY GET A MACRO OR SOMETHING!
    int a = nodestore_add_signal(ns, 0, 0, ROTATION);
    int b = nodestore_add_constant(ns, 0, 0, 180);
    int c = nodestore_add_predicate(ns, 75, 50, NEQ);
    Node* node_c = nodestore_get_node_by_id(ns, c);
    node_c->input.lhs = a;
    node_c->input.rhs = b;

    int d = nodestore_add_signal(ns, 0, 0, ROTATION);
    int e = nodestore_add_constant(ns, 0, 0, 0);
    int f = nodestore_add_predicate(ns, 450, 50, NEQ);
    Node* node_f = nodestore_get_node_by_id(ns, f);
    node_f->input.lhs = d;
    node_f->input.rhs = e;

    int g = nodestore_add_signal(ns, 0, 0, POS_Y);
    int h = nodestore_add_constant(ns, 0, 0, 600);
    int i = nodestore_add_predicate(ns, 200, 50, GT);
    Node* node_i = nodestore_get_node_by_id(ns, i);
    node_i->input.lhs = g;
    node_i->input.rhs = h;

    int j = nodestore_add_signal(ns, 0, 0, POS_Y);
    int k = nodestore_add_constant(ns, 0, 0, 100);
    int l = nodestore_add_predicate(ns, 325, 50, LT);
    Node* node_l = nodestore_get_node_by_id(ns, l);
    node_l->input.lhs = j;
    node_l->input.rhs = k;

    int m = nodestore_add_gate(ns, 125, 125, AND);
    Node* node_m = nodestore_get_node_by_id(ns, m);
    node_m->input.lhs = c;
    node_m->input.rhs = i;

    int n = nodestore_add_gate(ns, 325, 125, AND);
    Node* node_n = nodestore_get_node_by_id(ns, n);
    node_n->input.lhs = f;
    node_n->input.rhs = l;

    int o = nodestore_add_gate(ns, 100, 225, OR);
    Node* node_o = nodestore_get_node_by_id(ns, o);
    node_o->input.lhs = m;
    node_o->input.rhs = n;

    int p = nodestore_add_thruster(ns, 75, 325, BP);
    Node* node_p = nodestore_get_node_by_id(ns, p);
    node_p->parent = o;

    int q = nodestore_add_thruster(ns, 175, 325, SS);
    Node* node_q = nodestore_get_node_by_id(ns, q);
    node_q->parent = o;

    int r = nodestore_add_gate(ns, 375, 325, NOT);
    Node* node_r = nodestore_get_node_by_id(ns, r);
    node_r->parent = m;

    int s = nodestore_add_gate(ns, 475, 325, NOT);
    Node* node_s = nodestore_get_node_by_id(ns, s);
    node_s->parent = n;

    int t = nodestore_add_gate(ns, 435, 425, AND);
    Node* node_t = nodestore_get_node_by_id(ns, t);
    node_t->input.lhs = r;
    node_t->input.rhs = s;

    int u = nodestore_add_thruster(ns, 425, 525, BOOST);
    Node* node_u = nodestore_get_node_by_id(ns, u);
    node_u->parent = t;
}


static void*
game_setup(NVGcontext* vg)
{
    log_info("Setting up game");
    GameState* state = calloc(1, sizeof(GameState));

    // @TODO: If you have more than one font you need to store a
    // reference to this.
    nvgCreateFont(vg,
            "basic",
            "SourceSansPro-Regular.ttf");

    nodestore_init(&state->node_store, 5);

    /* NodeStore* ns = &state->node_store; */
    /* nodestore_load_test_nodes(ns); */

    state->player_ship.position.x = 300;
    state->player_ship.position.y = 99;
    state->player_ship.rotation = 0;

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
        
        log_info("adding node");
    }

    if (gui_button(state->gui, 70, 10, 50, 25)) {
        nodestore_add_gate(&state->node_store, 10, 45, AND);
        
        log_info("adding node");
    }

    if (gui_button(state->gui, 130, 10, 50, 25)) {
        nodestore_add_thruster(&state->node_store, 10, 45, BOOST);
        
        log_info("adding node");
    }

    // @TODO: It is very unfortunate that rendering happens in here....
    NodeEvent event = gui_nodes(&state->gui, &state->node_store);
    switch(event.type) {
    case NE_NAH:
        break;
    }

    // @TODO: decide if you want a pause.
    state->running = true;

    // Reset button
    // @TODO: reset the whole level not just the ship.
    if (gui_button_with_text(state->gui, 660, 2.5, 10, 5, "Reset")) {
            state->player_ship.position.x = 300;
            state->player_ship.position.y = 99;
            state->player_ship.rotation = 0;
    }

    // Update rockets.
    if (state->running) {
        Thrusters new_thrusters = nodestore_eval_thrusters(&state->node_store,
                                                           &state->player_ship);
        state->player_ship.thrusters = new_thrusters;
        ship_move(&state->player_ship, dt);
    }

    // Render

    // Space background!
    nvgBeginPath(vg);
    nvgRect(vg, 660, 10, 600, 700);
    nvgFillColor(vg, nvgRGBf(0.25, 0.25, 0.25));
    nvgFill(vg);

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
    }
    nvgRestore(vg);

    // @TODO: pull this out, this is hella useful.
    // debug print
    char buf[64] = {'\0'};
    snprintf(buf, 64, "position: (%f, %f), rotation: %d",
             state->player_ship.position.x,
             state->player_ship.position.y,
             state->player_ship.rotation);

    debug_draw_text(vg, 10, SCREEN_HEIGHT - 50, 24, buf);
}


const gg_Game gg_game_api = {
        .init = game_setup,
        .update_and_render = game_update_and_render
};


char*
bool_string(bool b)
{
    return b ? "true" : "false";
}
