#include "gameguy.h"
#include "game.h"

static const int X_PADDING = 12.0;
static const int Y_PADDING = 12.0;

static NVGcontext* gg_Debug_vg = NULL;

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


// Rendering Code
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
nodestore_render(NVGcontext* vg, NodeStore* ns)
{
    // Draw Nodes
    for (int i = 0; i < ns->next_id; i++) {
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

            nvgSave(vg);
            {
                nvgTranslate(vg, node.position.x, node.position.y);
                draw_ship(vg, thrusts, true);
            }
            nvgRestore(vg);
            
            draw_parent_line(vg, &node, nodestore_get_node_by_id(ns, node.parent));
        } break;

        case PREDICATE: {

            Node* lhs = nodestore_get_node_by_id(ns, node.input.lhs);
            Node* rhs = nodestore_get_node_by_id(ns, node.input.rhs);
            node_get_text(&node, buf, 256, lhs, rhs);
            draw_text_box(vg, buf, node.position.x, node.position.y);
        } break;

        case SIGNAL:
            break;

        case CONSTANT:
            break;

        case GATE: {
            node_get_text(&node, buf, 256, NULL, NULL);
            draw_text_box(vg, buf, node.position.x, node.position.y);
            draw_parent_line(vg, &node, nodestore_get_node_by_id(ns, node.input.rhs));
            draw_parent_line(vg, &node, nodestore_get_node_by_id(ns, node.input.lhs));
        } break;

        }
    }
}


void debug_square(NVGcontext* vg, float x, float y)
{
    nvgBeginPath(vg);
    nvgRect(vg, x, y, 1, 1);
    nvgFill(vg);
}


void debug_text(NVGcontext* vg, float x, float y, int size, const char* txt)
{
    nvgSave(vg);
    nvgFontSize(vg, size);
    nvgFillColor(vg, nvgRGBf(1, 1, 1));
    nvgText(vg, x, y, txt, NULL);
    nvgRestore(vg);
}


void ship_move(Ship* ship, float dt)
{
    V2 force = {0.0, 0.0};
    int rotation = 0;

    if (ship->thrusters.bp) {
        force = v2_plus(force, (V2){1, 0});
        rotation -= 1;
    }

    if (ship->thrusters.bs) {
        force = v2_plus(force, (V2){-1, 0});
        rotation += 1;
    }

    if (ship->thrusters.sp) {
        force = v2_plus(force, (V2){1, 0});
        rotation += 1;
    }

    if (ship->thrusters.ss) {
        force = v2_plus(force, (V2){-1, 0});
        rotation -= 1;
    }

    if (ship->thrusters.boost) {
        force = v2_plus(force, (V2){0, 5});
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


int ship_get_signal(const Ship* ship, Signal signal)
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


int node_eval_sub_node(const Node* node, const Ship* ship)
{
    if (node->type == SIGNAL) {
        return ship_get_signal(ship, node->signal);
    } else { // Constant
        return node->constant;
    }
}


bool node_eval(const Node* node, const NodeStore* ns, const Ship* ship)
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

        // todo(stephen): IF DEBUG
        nvgSave(gg_Debug_vg);
        {
            if (value) {
                nvgFillColor(gg_Debug_vg, nvgRGBf(0,1,0));
            } else {
                nvgFillColor(gg_Debug_vg, nvgRGBf(1,0,0));
            }
            debug_square(gg_Debug_vg, node->position.x-25, node->position.y);
        }
        nvgRestore(gg_Debug_vg);

        if (node->type == THRUSTER) {
            switch(node->thruster) {
            case BP:
                out_thrusters.bp = out_thrusters.bp || value;
                break;
            case BS:
                out_thrusters.bs = out_thrusters.bs || value;
                break;
            case SP:
                out_thrusters.sp = out_thrusters.sp || value;
                break;
            case SS:
                out_thrusters.ss = out_thrusters.ss || value;
                break;
            case BOOST:
                out_thrusters.boost = out_thrusters.boost || value;
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

    state->drag_target = -1;

    // todo(Stephen): If you have more than one font you need to store a
    // reference to this.
    nvgCreateFont(vg,
            "basic",
            "SourceSansPro-Regular.ttf");

    nodestore_init(&state->node_store, 5);
    NodeStore* ns = &state->node_store;

    nodestore_load_test_nodes(ns);

    state->player_ship.position.x = 300;
    state->player_ship.position.y = 99;
    state->player_ship.rotation = 0;


    /* nodestore_add_gate(ns, 100, 125, AND); */
    /* nodestore_add_thruster(ns, 425, 575, BOOST); */
    /* nodestore_add_gate(ns, 100, 225, NOT); */
    /* nodestore_add_gate(ns, 100, 325, NOT); */
    /* nodestore_add_gate(ns, 100, 425, NOT); */
    /* nodestore_add_gate(ns, 100, 525, NOT); */
    /* nodestore_add_gate(ns, 100, 625, NOT); */
    /* nodestore_add_gate(ns, 100, 725, NOT); */
    /* nodestore_add_gate(ns, 100, 825, NOT); */
    /* nodestore_add_gate(ns, 100, 925, NOT); */

    return state;
}

static void
game_update_and_render(void* gamestate,
                       NVGcontext* vg,
                       gg_Input input,
                       float dt)
{
    if (!gg_Debug_vg) {
        gg_Debug_vg = vg;
    }
    GameState* state = (GameState*)gamestate;

    // Prepare GUI state todo(stephen): move this to a function.
    state->gui.vg = vg;
    state->gui.mouse_x = input.mouse_x;
    state->gui.mouse_y = input.mouse_y;
    state->gui.click = input.click;

    if (gui_button(state->gui, 100, 100, 100, 100)) {
        log_info("clicked");
    }

#if 0

    // Update    
    // todo(stephen): Maybe pass the world to this depending on what the signals
    // end up being.
    Thrusters new_thrusters = nodestore_eval_thrusters(&state->node_store,
                                                       &state->player_ship);
    state->player_ship.thrusters = new_thrusters;
    ship_move(&state->player_ship, dt);

    // handle draging
    // todo(stephen): Drag target can change when moving over one earlier in
    // the array. Need to tag currently being dragged node and just drag that
    // until dragging ends.
    if (input.end_dragging) {
        state->drag_target = -1;
    }
    
    if (input.is_dragging && input.mouse_motion) {
        Node* node;

        // Pick node to drag.
        if (state->drag_target == -1) {
            for (int i = 0; i < state->node_store.next_id; i++) {
                node = nodestore_get_node_by_id(&state->node_store, i);
                if (node->type != CONSTANT && node->type != SIGNAL) {
                    BoundingBox new_bb =
                        node_calc_bounding_box(vg, node, &state->node_store);

                    if (bb_contains(new_bb,
                                    input.mouse_x - input.mouse_xrel,
                                    input.mouse_y - input.mouse_yrel)) {
                        state->drag_target = i;
                        break;
                    }
                }
            }
        } else {
            node = nodestore_get_node_by_id(&state->node_store, state->drag_target);
        }

        node->position.x += input.mouse_xrel;
        node->position.y += input.mouse_yrel;
    }

    // buttons for creating new nodes.
    // then I also need ways to edit existing nodes.
    // And I need to connect and un connect nodes....

    

    
    // Render

    // Space scene!
    nvgBeginPath(vg);
    nvgRect(vg, 660, 10, 600, 700);
    /* nvgRect(vg, 1000, 25, 1000, 1500); */
    nvgFillColor(vg, nvgRGBf(0.25, 0.25, 0.25));
    nvgFill(vg);

    // These probably need some ui and stuff around them.
    nodestore_render(vg, &state->node_store);

    nvgSave(vg);
    {
        // x,y positions will need their y flipped to be drawn in the proper
        // place. The translates take care of the rest of the movement. Math in
        // space can be done in cartesian coordinates, just need this work to
        // draw in the right place. Drawing will be done in normal nvg
        // coordinates so stuff like text works. Collision detection will have
        // to be in cartesian and seperate from rendering.
        
        /* nvgTranslate(vg, 1000, 25); */
        /* nvgTranslate(vg, 0, 1500); */
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
            /* nvgTranslate(vg, node->position.x, node->position.y); */
            /* draw_ship(vg, 0.0, 0.0, thrusts, true); */
        }
        nvgRestore(vg);
        
        // debug rects!
        /* nvgFillColor(vg, nvgRGBf(0, 1000, 1000)); */
        /* debug_square(vg, 1000, -1500); */
        
    }
    nvgRestore(vg);

    // debug print
    char buf[64] = {'\0'};
    snprintf(buf, 64, "position: (%f, %f), rotation: %d",
             state->player_ship.position.x,
             state->player_ship.position.y,
             state->player_ship.rotation);

    debug_text(vg, 10, SCREEN_HEIGHT - 50, 24, buf);
#endif

}


const gg_Game gg_game_api = {
        .init = game_setup,
        .update_and_render = game_update_and_render
};


char* bool_string(bool b)
{
    return b ? "true" : "false";
}
