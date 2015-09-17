#ifndef _rockets_nodes_h
#define _rockets_nodes_h

typedef enum { SENSOR, PREDICATE, GATE, THRUSTER } NodeType;

typedef struct {

} SensorNode;

typedef struct {

} PredicateNode;

typedef struct {

} GateNode;

typedef struct {

} ThrusterNode;

// @NOTE: Using a union for this makes the memory management really easy with 1 freelist.
typedef struct node {
    int id;
    NodeType type;
    V2 position;

    union {
        SensorNode sensor;
        PredicateNode predicate;
        GateNode gate;
        ThrusterNode thruster;
    };

    struct node* next_in_hash;
} Node;

typedef struct {
    int count;
    int last_used_id;
    Node node_buffer[512]; // @TODO: Could pull a platform dependent size from an arena.
    Node* first_free_node;

    // @NOTE: Must be power of 2 for bad hash function.
    Node* id_hash[128];

    // @TODO: Add another index for the topological sort. Makes evaluation faster.
} NodeStore;

#endif
