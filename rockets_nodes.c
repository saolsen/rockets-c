// @NOTE: This is just for flymake. Unity build has it already.
#include "rockets.h"

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
