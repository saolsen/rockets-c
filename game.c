#include "gameguy.h"
#include "game.h"

// Rendering Code
void
draw_text_box(NVGcontext* vg, const char* txt, float x, float y)
{
    nvgSave(vg);
    {
        // Setup Text
        nvgFontSize(vg, 24);

        // Get Text Bounds
        float bounds[4];
        nvgTextBounds(vg, x, y, txt, NULL, bounds);

        float x_padding = 12.0;
        float y_padding = 12.0;

        // Draw Background
        nvgSave(vg);
        {
            nvgBeginPath(vg);
            nvgRect(vg,
                    bounds[0] - x_padding,
                    bounds[1] - y_padding,
                    bounds[2] - bounds[0] + 2 * x_padding,
                    bounds[3] - bounds[1] + 2 * y_padding);
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


// todo(stephen): Have a type of thing to draw for selecting a new node.
//                used during creation.
void
node_draw_gate(NVGcontext* vg, Node node)
{
    // todo(stephen): Show the connection points.


    // todo(stephen): Assert that the node type is gate.
    const char* txt = NULL;

    switch(node.gate) {
        case AND: {
            txt = "AND";
        } break;
        case OR: {
            txt = "OR";
        } break;
        case NOT: {
            txt = "NOT";
        } break;
    }

    draw_text_box(vg, txt, node.position.x, node.position.y);
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

        // Center (for debugging)
        /* nvgBeginPath(vg); */
        /* nvgCircle(vg, x, y, 1.0); */
        /* nvgFillColor(vg, nvgRGBf(1.0, 1.0, 1.0)); */
        /* nvgFill(vg); */
    }

    nvgRestore(vg);
}


void
node_draw_thruster(NVGcontext* vg, Node* node)
{
    // TODO(stephen): draw bounding box
    Thrusters thrusts = {};
    switch(node->thruster) {
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

    nvgSave(vg);
    {
        nvgTranslate(vg, node->position.x, node->position.y);
        draw_ship(vg, thrusts, true);
    }
    nvgRestore(vg);

}


void
node_get_text(Node* node, char* buffer)
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
        char str[15];
        sprintf(str, "%d", node->signal);
        txt = str;
        break;
    } break;
    case PREDICATE: {
        switch(node->predicate) {
        case LT:
            txt = "<";
            break;
        case GT:
            txt = ">";
            break;
        case LEQT:
            txt = "<=";
            break;
        case GEQT:
            txt = ">=";
            break;
        case EQ:
            txt = "==";
            break;
        case NEQ:
            txt = "<>";
            break;
        }
    } break;
    default:
        break;
    }
    
    strcpy(buffer, txt);
}

void
node_draw_predicate(NVGcontext* vg, Node* node, Node* lhs, Node* rhs)
{
    char buf[256] = {'\0'};
    char str[15] = {'\0'};
    size_t max_len = sizeof buf - 1;
 
    if (NULL != lhs) {
        node_get_text(lhs, str);
        /* log_info("lhs_text: %s", str); */
        strncat(buf, str, max_len);
        strncat(buf, " ", max_len - strlen(buf));
        str[0] = '\0';
    }
 
    node_get_text(node, str);
    /* log_info("pred_text: %s", str); */
    strncat(buf, str, max_len - strlen(buf));
    str[0] = '\0';
 
    if (NULL != rhs) {
        node_get_text(rhs, str);
        /* log_info("rhs_text: %s", str); */
        strncat(buf, " ", max_len- strlen(buf));
        strncat(buf, str, max_len - strlen(buf));
        str[0] = '\0';
    }
 
    /* log_info("buffer: %s", buf); */
 
    draw_text_box(vg, buf, node->position.x, node->position.y);
}


void
nodestore_render(NVGcontext* vg, NodeStore* ns)
{
    // Draw Nodes
    for (int i = 0; i < ns->next_id; i++) {
        Node node = ns->array[i];

        switch(node.type) {
            case THRUSTER:
                node_draw_thruster(vg, &node);
                break;

            case PREDICATE: {
                Node* lhs = nodestore_get_node_by_id(ns, node.input.lhs);
                Node* rhs = nodestore_get_node_by_id(ns, node.input.rhs);
                node_draw_predicate(vg, &node, lhs, rhs);
            } break;

            case SIGNAL:
                break;

            case CONSTANT:
                break;

            case GATE:
                node_draw_gate(vg, node);
                break;

        }
    }

    // Draw Connections
    // todo(stephen)
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

    int n = nodestore_add_gate(ns, 125, 125, AND);
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

    // todo(Stephen): If you have more than one font you need to store a
    // reference to this.
    nvgCreateFont(vg,
            "basic",
            "SourceSansPro-Regular.ttf");

    nodestore_init(&state->node_store, 5);
    NodeStore* ns = &state->node_store;

    nodestore_load_test_nodes(ns);

    state->player_ship.position.x = 300;
    state->player_ship.position.y = 300;
    state->player_ship.rotation = 0.0;

    /* state->player_ship.thrusters.bp = true; */
    /* state->player_ship.thrusters.bs = false; */
    /* state->player_ship.thrusters.sp = false; */
    /* state->player_ship.thrusters.ss = false; */
    /* state->player_ship.thrusters.boost = true; */

    return state;
}

void debug_square(NVGcontext* vg, float x, float y)
{
    nvgBeginPath(vg);
    nvgRect(vg, x, y, 1, 1);
    nvgFill(vg);
}

void ship_move(Ship* ship, float dt)
{
    V2 force = {0.0, 0.0};
    float rotation = 0.0;

    if (ship->thrusters.bp) {
        force = v2_plus(force, (V2){1, 0});
        rotation -= 1.0;
    }

    if (ship->thrusters.bs) {
        force = v2_plus(force, (V2){-1, 0});
        rotation += 1.0;
    }

    if (ship->thrusters.sp) {
        force = v2_plus(force, (V2){1, 0});
        rotation += 1.0;
    }

    if (ship->thrusters.ss) {
        force = v2_plus(force, (V2){-1, 0});
        rotation -= 1.0;
    }

    if (ship->thrusters.boost) {
        force = v2_plus(force, (V2){0, 5});
    }

    V2 abs_force = v2_rotate(force, deg_to_rad(ship->rotation));
    float speed = 50;

    /* log_info("force: %f, %f", force.x, force.y); */

    ship->position.x += abs_force.x*speed*dt;
    ship->position.y += abs_force.y*speed*dt;
    ship->rotation = (int)(ship->rotation + rotation) % 360;

}

int ship_get_signal(const Ship* ship, Signal signal)
{
    switch(signal) {
    case POS_X:
        return 1;
        break;
    case POS_Y:
        return 2;
        break;
    case ROTATION:
        return 3;
        break;
    }

    //error
    return -1;
}
// You guys, polymorphic functions would be pretty damn useful right here!

// todo(stephen): Figure out how the hell to write this without
// recursion or maps.
int node_eval_sub_node(const Node* node, const Ship* ship)
{
    if (node->type == SIGNAL) {
        return ship_get_signal(ship, node->signal);
    } else { // Constant
        return node->constant;
    }
}

// note(stephen): This isn't a full function, I can't use it for signal and constant
// because those nodes have non-bool values.
bool node_eval(const Node* node, const NodeStore* ns, const Ship* ship)
{
    switch(node->type) {
    case SIGNAL: {
        // only a sub node
    } break;
    case CONSTANT: {
        // only a sub node
    } break;
    case PREDICATE: {
        int lhs = node_eval_sub_node(nodestore_get_node_by_id(ns, node->input.lhs), ship);
        int rhs = node_eval_sub_node(nodestore_get_node_by_id(ns, node->input.rhs), ship);

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
        bool lhs = node_eval(nodestore_get_node_by_id(ns, node->input.lhs), ns, ship);
        bool rhs = node_eval(nodestore_get_node_by_id(ns, node->input.lhs), ns, ship);

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
    Thrusters out_thrusters = {false, false, false, false, true};

    // iterate over thruster nodes
    for (int i = 0; i < ns->next_id; i++) {
        Node* node = nodestore_get_node_by_id(ns, i);
        bool value = node_eval(node, ns, ship);

        if (node->type == THRUSTER) {
            switch(node->thruster) {
            case BP:
                out_thrusters.bp = value;
                break;
            case BS:
                out_thrusters.bs = value;
                break;
            case SP:
                out_thrusters.sp = value;
                break;
            case SS:
                out_thrusters.ss = value;
                break;
            case BOOST:
                out_thrusters.boost = value;
                break;
            }
        }
    }
    
    return out_thrusters;
}


static void
game_update_and_render(void* gamestate,
                       NVGcontext* vg,
                       gg_Input input,
                       float dt)
{
    GameState* state = (GameState*)gamestate;

    // Update
    
    ship_move(&state->player_ship, dt);

    // Render

    // todo(stephen): I need to translate to the place I really want to render
    // this stuff before drawing.

    // Space scene!
    nvgBeginPath(vg);
    nvgRect(vg, 1000, 25, 1000, 1500);
    nvgFillColor(vg, nvgRGBf(0.25, 0.25, 0.25));
    nvgFill(vg);

    // These probably need some ui and stuff around them.
    nodestore_render(vg, &state->node_store);

    // todo(stephen): Maybe pass the world to this depending on what the signals
    // end up being.
    Thrusters new_thrusters = nodestore_eval_thrusters(&state->node_store,
                                                       &state->player_ship);
    state->player_ship.thrusters = new_thrusters;

    nvgSave(vg);
    {
        // x,y positions will need their y flipped to be drawn in the proper place.
        // the translates take care of the rest of the movement. Math in space can
        // be done in cartesian coordinates, just need this work to draw in the
        // right place. Drawing will be done in normal nvg coordinates so stuff like
        // text works. Collision detection will have to be in cartesian and seperate
        // from rendering.
        nvgTranslate(vg, 1000, 25);
        nvgTranslate(vg, 0, 1500);

        nvgSave(vg);
        {
            nvgTranslate(vg,
                         state->player_ship.position.x,
                         -state->player_ship.position.y);
            nvgRotate(vg, -deg_to_rad(state->player_ship.rotation));
            draw_ship(vg,
                      state->player_ship.thrusters,
                      false);
            /* nvgTranslate(vg, node->position.x, node->position.y); */
            /* draw_ship(vg, 0.0, 0.0, thrusts, true); */
        }
        nvgRestore(vg);

        
        // debug rects!
        /* nvgFillColor(vg, nvgRGBf(0, 1000, 1000)); */
        /* debug_square(vg, 1000, -1500); */
        
    }
    nvgRestore(vg);
}


const gg_Game gg_game_api = {
        .init = game_setup,
        .update_and_render = game_update_and_render
};
