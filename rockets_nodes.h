#ifndef _rockets_nodes_h
#define _rockets_nodes_h

typedef enum { SENSOR, CONSTANT, PREDICATE, GATE, THRUSTER } NodeType;
struct node;

typedef struct {
    Direction sensor_direction;
    // I think sensor nodes are going to be entity specific.
    int entity_id;
} SensorNode;

typedef struct {
    int value;
} ConstantNode;

typedef enum { EQ=0, NEQ=1, LT=2, GT=3, LEQ=4, GEQ=5 } Predicate;

typedef struct {
    Predicate predicate;
    struct node* lhs;
    struct node* rhs;
} PredicateNode;

typedef enum { AND=0, OR=1, NOT=2 } Gate;

typedef struct {
    Gate gate;
    struct node* lhs;
    struct node* rhs;
} GateNode;

typedef enum {
    BP    = (1 << 0), // 1
    BS    = (1 << 1), // 2
    SP    = (1 << 2), // 4
    SS    = (1 << 3), // 8
    BOOST = (1 << 4), // 16
} Thruster;

typedef struct {
    Thruster thruster;
    struct node* input;
} ThrusterNode;

#define NAH -1

// @NOTE: Using a union for this makes the memory management really easy with 1 freelist.
typedef struct node {
    int id;
    NodeType type;
    V2 position;

    union {
        SensorNode sensor;
        ConstantNode constant;
        PredicateNode predicate;
        GateNode gate;
        ThrusterNode thruster;
    };

    int current_value; // yo if this is -1 then NAHHHHH

    struct node* next_in_hash;
} Node;

typedef struct {
    int last_used_id;
    
    int node_buffer_size;
    Node* node_buffer_base;
    int node_buffer_used;
    Node* first_free_node;

    // @NOTE: Must be power of 2 for bad hash function.
    Node* id_hash[128];

    // @TODO: Add another index for the topological sort. Makes evaluation faster.
} NodeStore;

#endif
