#ifndef _rockets_nodes_h
#define _rockets_nodes_h

// @NOTE: These are just for flymake, real build doesn't need them.
// #include "rockets_math.h"

typedef enum { THRUSTER, PREDICATE, SIGNAL, CONSTANT, GATE } NodeType;
typedef enum { BP, BS, SP, SS, BOOST }                       Thruster;
typedef enum { LT, GT, LEQT, GEQT, EQ, NEQ }                 Predicate;
typedef enum { POS_X, POS_Y, ROTATION }                      Signal;
typedef enum { AND, OR, NOT }                                Gate;

// @OPTOMIZE: Compress this to a bit array.
typedef struct Thrusters {
    bool bp;
    bool bs;
    bool sp;
    bool ss;
    bool boost;
} Thrusters;

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

    struct Node* next_in_hash;
} Node;

typedef struct {
    int count;
    int last_id; // last_id because we start with 1, increment before using.
    Node nodes[256];
    Node* first_free_node;

    // @NOTE: Must be power of 2 for bad hash function.
    Node* id_hash[128];

    // @TODO: Add another index for the topological sort. Makes evaluation faster.
} NodeStore;

#endif
