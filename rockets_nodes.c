#include "rockets.h"


uint32_t
node_id_hash(NodeStore* ns, int id)
{
    // @TODO: Better hash function.
    uint32_t hash_bucket = id & (ARRAY_COUNT(ns->id_hash) - 1);
    return hash_bucket;
}


NodeStore*
nodestore_allocate(MemoryArena* arena)
{
    NodeStore* ns = arena_push_struct(arena, NodeStore);
    ns->node_buffer_base = arena_push_size(arena, sizeof(Node) * 512);
    
    ns->node_buffer_size = 512;
    ns->node_buffer_used = 0;
    ns->last_used_id = 0;
    ns->first_free_node = NULL;

    for (int i=0; i<ARRAY_COUNT(ns->id_hash); i++) {
        ns->id_hash[i] = NULL;
    }

    return ns;
}


Node*
nodestore_push_node(NodeStore* ns, NodeType type)
{
    Node* new_node = NULL;
    if (ns->first_free_node) {
        new_node = ns->first_free_node;
        ns->first_free_node = new_node->next_in_hash;
    } else {
        assert(ns->node_buffer_used < ns->node_buffer_size);
        new_node = ns->node_buffer_base + ns->node_buffer_used++;
    }

    int id = ++ns->last_used_id;

    // Add to hash.
    uint32_t hash_bucket = node_id_hash(ns, id);
    new_node->next_in_hash = ns->id_hash[hash_bucket];
    ns->id_hash[hash_bucket] = new_node;

    new_node->id = id;
    new_node->type = type;

    switch(type) {
    case(SENSOR): {
        new_node->sensor.sensor_direction = UP;
        
    } break;
    case(PREDICATE): {
        new_node->predicate.predicate = EQ;
        
    } break;
    case(GATE): {
        
        
    } break;
    case(THRUSTER): {
        
        
    } break;
    }

    return new_node;
}


Node*
nodestore_get_node_by_id(NodeStore* ns, int id)
{
    if (id == 0) return NULL;

    assert(id > 0);
    assert(id <= ns->last_used_id);

    // @TODO: Better hash function.
    uint32_t hash_bucket = node_id_hash(ns, id);
    for (Node* node = ns->id_hash[hash_bucket];
         node;
         node = node->next_in_hash) {
        if (node->id == id) {
            return node;
        }
    }

    return NULL;
}


void
nodestore_delete_node(NodeStore* ns, int id)
{
    assert(id <= ns->last_used_id);
    assert(id > 0);

    // Delete node.
    uint32_t hash_bucket = node_id_hash(ns, id);
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

    // Zero out any pointers to id.
    // @TODO: Do this once we know the structure of the nodes.
    for (int i=0; i<ns->node_buffer_used; i++) {
        Node* node = ns->node_buffer_base + i;
        if (node->id == 0) continue;
        switch(node->type) {
        case(SENSOR): {
        
        
        } break;
        case(PREDICATE): {
        
        
        } break;
        case(GATE): {
        
        
        } break;
        case(THRUSTER): {
        
        
        } break;
        }
    }
}
