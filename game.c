#include "gameguy.h"

// For now I've been using malloc so I need to make sure this lib is dynamically linked
// against stdlib not statically so that the game code will still own all the memory.
// In the future I should look into doing things the handmade hero way and slicing up
// a chunk of preallocated memory myself.

typedef struct {
    float x;
    float y;
} Point;


typedef enum { THRUSTER, PREDICATE, SIGNAL, CONSTANT, GATE } NodeType;
typedef enum { BP, BS, SP, SS, BOOST }                       Thruster;
typedef enum { LT, GT, LEQT, GEQT, EQ, NEQ }                 Predicate;
typedef enum { POS_X, POS_Y, ROTATION }                      Signal;
typedef enum { AND, OR, NOT }                                Gate;


// todo(stephen): Because the links are int they will start as 0 which is
//                bad because then I'll have all elements linking to the 0th.
//                Need a value like -1 or something for null.

// todo(stephen): Typedef the id so I can change it later.
typedef struct Node {
    int id;
    NodeType type;
    Point position;

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

// Functions needed to interact with nodes.
// Create and Remove Nodes.
// Move nodes around.
// Connect nodes to eachother.
// Render nodes.


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

    nvgSave(vg);
    {
        // Setup Text
        nvgFontSize(vg, 24);

        // Get Text Bounds
        float bounds[4];
        nvgTextBounds(vg, node.position.x, node.position.y, txt, NULL, bounds);

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
        nvgText(vg, node.position.x, node.position.y, txt, NULL);
    }
    nvgRestore(vg);
}


void
node_draw_thruster(NVGcontext* vg, Node* node)
{

}


void
node_draw_predicate(NVGcontext* vg, Node* node, Node* lhs, Node* rhs)
{
    // check if lhs or rhs are NULL.


}


// Id is just index, this doesn't work when I want to remove elements
// todo(stephen): Revisit this.
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
nodestore_get_node_by_id(NodeStore* ns, int id)
{
    if (id > 0 && id < ns->next_id) {
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


void
nodestore_render(NVGcontext* vg, NodeStore* ns)
{
    // Draw Nodes
    for (int i = 0; i < ns->next_id; i++) {
        Node node = ns->array[i];

        switch(node.type) {
            case THRUSTER:
                break;

                // This might be a cache buster, revisit.
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
    /* int constant = nodestore_add_constant(ns, 100, 100, 12); */
    /* /\* Node* constant_n = nodestore_get_node_by_id(ns, constant); *\/ */
    /* int signal = nodestore_add_signal(ns, 200, 200, POS_X); */
    /* /\* Node* signal_n = nodestore_get_node_by_id(ns, signal); *\/ */
    /* int pred = nodestore_add_predicate(ns, 300, 300, LT); */
    /* Node* pred_n = nodestore_get_node_by_id(ns, pred); */
    /* int not_gate = nodestore_add_gate(ns, 700, 200, NOT); */
    /* Node* not_n = nodestore_get_node_by_id(ns, not_gate); */

    /* pred_n->input.lhs = constant; */
    /* pred_n->input.rhs = signal; */

    /* not_n->parent = pred; */
    int a = nodestore_add_signal(ns, 0, 0, ROTATION);
    int b = nodestore_add_constant(ns, 0, 0, 180);
    int c = nodestore_add_predicate(ns, 75, 50, NEQ);

    int d = nodestore_add_signal(ns, 0, 0, ROTATION);
    int e = nodestore_add_constant(ns, 0, 0, 180);
    int f = nodestore_add_predicate(ns, 75, 50, NEQ);

    int g = nodestore_add_gate(ns, 100, 100, AND);
}


typedef struct {
    NodeStore node_store;

} GameState;


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

    // todo(stephen): Probably pass x and y to add_gate;

    nodestore_load_test_nodes(ns);

    return state;
}


static void
game_update_and_render(void* gamestate,
                       NVGcontext* vg,
                       gg_Input input,
                       float dt)
{
    GameState* state = (GameState*)gamestate;
    


    nodestore_render(vg, &state->node_store);
    /* nvgBeginPath(vg); */
    /* nvgRect(vg, 10.0, 10.0, 100.0, 100.0); */
    /* nvgFillColor(vg, nvgRGB(1.0, 0.0, 0.0)); */
    /* nvgFill(vg); */
    /* nvgBeginPath(vg); */

    /* nvgFill(vg); */

}


const gg_Game gg_game_api = {
        .init = game_setup,
        .update_and_render = game_update_and_render
};
