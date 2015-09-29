#ifndef _rockets_nodes_h
#define _rockets_nodes_h

typedef enum { SENSOR=0, CONSTANT=1, PREDICATE=2, GATE=3, THRUSTER=4 } NodeType;

char* nodetype_names[] = {
    "SENSOR",
    "CONSTANT",
    "PREDICATE",
    "GATE",
    "THRUSTER",
};

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

char* predicate_names[] = {
    "==",
    "<>",
    "<",
    ">",
    "<=",
    ">="
};

typedef struct {
    Predicate predicate;
    struct node* lhs;
    struct node* rhs;
} PredicateNode;

typedef enum { AND=0, OR=1, NOT=2 } Gate;

char* gate_names[] = {
    "AND",
    "OR",
    "NOT"
};

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

typedef struct node {
    int id;
    NodeType type;
    V2 position;

    // @NOTE: Using a union for this makes the memory management really easy with 1 freelist.
    union {
        SensorNode sensor;
        ConstantNode constant;
        PredicateNode predicate;
        GateNode gate;
        ThrusterNode thruster;
    };

    int current_value; // yo if this is -1 then NAHHHHH

    struct node* next_in_hash;

    // Keep track of number of nodes that depend on this to make topological iteration easier.
    int num_dependencies;
    struct node* next_in_q;
    
} Node;

// @NOTE: I'm not sure I really need the id or the hash.
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
