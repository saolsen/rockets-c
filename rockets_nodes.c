// @BUG: There are some real bugs in node evaluation here. I don't have a debugger though so I should display
//       the current value of the nodes below them.

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
    new_node->current_value = NAH;

    switch(type) {
    case(SENSOR): {
        new_node->sensor.sensor_direction = UP;
        new_node->sensor.entity_id = 0;
        
    } break;
    case(CONSTANT): {
        new_node->constant.value = 0;
        
    } break;
    case(PREDICATE): {
        new_node->predicate.predicate = EQ;
        new_node->predicate.lhs = NULL;
        new_node->predicate.rhs = NULL;
        
    } break;
    case(GATE): {
        new_node->gate.gate= AND;
        new_node->gate.lhs = NULL;
        new_node->gate.rhs = NULL;
        
    } break;
    case(THRUSTER): {
        new_node->thruster.thruster = BP;
        new_node->thruster.input = NULL;
        
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
    Node* removed_node = NULL;
    
    uint32_t hash_bucket = node_id_hash(ns, id);
    for (Node** node = &ns->id_hash[hash_bucket];
         *node;
         node = &(*node)->next_in_hash) {
        if ((*node)->id == id) {
            removed_node = *node;
            *node = (*node)->next_in_hash;
            removed_node->next_in_hash = ns->first_free_node;
            removed_node->id = 0;
            ns->first_free_node = removed_node;
            break;
        }
    }

    assert(removed_node);

    // Zero out any pointers to id.
    // @TODO: Do this once we know the structure of the nodes.
    for (int i=0; i<ns->node_buffer_used; i++) {
        Node* node = ns->node_buffer_base + i;
        if (node->id == 0) continue;
        switch(node->type) {
        case(SENSOR): {        
            
        } break;
        case(CONSTANT): {

        } break;
        case(PREDICATE): {
            if (node->predicate.lhs == removed_node) {
                node->predicate.lhs = NULL;
            }
            if (node->predicate.rhs == removed_node) {
                node->predicate.rhs = NULL;
            }
        
        } break;
        case(GATE): {
            if (node->gate.lhs == removed_node) {
                node->gate.lhs = NULL;
            }
            if (node->gate.rhs == removed_node) {
                node->gate.rhs = NULL;
            }
        
        } break;
        case(THRUSTER): {
            if (node->thruster.input == removed_node) {
                node->thruster.input = NULL;
            }
        
        } break;
        }
    }
}


uint32_t
nodestore_eval(GameState* state)
{
    // Evaluate nodes based on their next_position as that is the position they will be moved to this frame.
    Node* top_q_begin = NULL;
    Node* top_q_end = NULL;
    
    // Build up the num_dependencies on each node.
    for (int i=0; i<state->node_store->node_buffer_used; i++) {
        Node* node = state->node_store->node_buffer_base + i;
        if (node->id == 0) continue;

        // @TODO: Is this the best way to determine this is the first valid node?
        if (!(top_q_begin && top_q_end)) {
            top_q_begin = node;
            top_q_end = node;
        } else {
            top_q_end->next_in_q = node;
            top_q_end = node;    
        }
        
        node->next_in_q = NULL;
        node->num_dependencies = 0;
    }
    
    for (int i=0; i<state->node_store->node_buffer_used; i++) {
        Node* node = state->node_store->node_buffer_base + i;
        if (node->id == 0) continue;

        switch(node->type) {
        case(SENSOR): {
            // nah
        } break;
        case(CONSTANT): {
            // nah
        } break;
        case(PREDICATE): {
            if (node->predicate.lhs) {
                node->predicate.lhs->num_dependencies++;
            }
            if (node->predicate.rhs) {
                node->predicate.rhs->num_dependencies++;
            }
        } break;
        case(GATE): {
            if (node->gate.lhs) {
                node->gate.lhs->num_dependencies++;
            }
            if (node->gate.rhs) {
                node->gate.rhs->num_dependencies++;
            }
        } break;
        case(THRUSTER): {
            if (node->thruster.input) {
                node->thruster.input->num_dependencies++;
            }
        } break;
        };
    }

    uint32_t thrusters = 0;

    // Iterate topologically.
    for (Node* node = top_q_begin;
        node;
        /* node = node->next_in_q */) {
        if (node->num_dependencies != 0) {
            // Put back on queue.
            top_q_end->next_in_q = node;
            Node* next_in_q = node->next_in_q;
            node->next_in_q = NULL;
            top_q_end = node;
            node = next_in_q;
        } else {
            // Evaluate.
            switch(node->type) {
            case(SENSOR): {
                // @TODO: Calculate sensor value;
                // This is still kinda sketch so lets just start with the goal.
                node->current_value = 1;
                
            } break;
            case(CONSTANT): {
                node->current_value = node->constant.value;
                
            } break;
            case(PREDICATE): {
                if (node->predicate.lhs && node->predicate.rhs) {
                    switch(node->predicate.predicate) {
                    case(EQ):
                        node->current_value =
                            node->predicate.lhs->current_value == node->predicate.rhs->current_value;
                        break;
                    case(NEQ):
                        node->current_value =
                            node->predicate.lhs->current_value != node->predicate.rhs->current_value;
                        break;
                    case(LT):
                        node->current_value =
                            node->predicate.lhs->current_value < node->predicate.rhs->current_value;
                        break;
                    case(GT):
                        node->current_value =
                            node->predicate.lhs->current_value > node->predicate.rhs->current_value;
                        break;
                    case(LEQ):
                        node->current_value =
                            node->predicate.lhs->current_value <= node->predicate.rhs->current_value;
                        break;
                    case(GEQ):
                        node->current_value =
                            node->predicate.lhs->current_value >= node->predicate.rhs->current_value;
                        break;
                    };
                    
                } else {
                    node->current_value = false;
                }

                if (node->predicate.lhs) {
                    node->predicate.lhs->num_dependencies--;
                }
                if (node->predicate.rhs) {
                    node->predicate.rhs->num_dependencies--;
                }
                
            } break;
            case(GATE): {
                bool both = node->gate.lhs && node->gate.rhs;                
                switch(node->gate.gate) {
                case(AND):
                    if (both) {
                        node->current_value = 
                            node->gate.lhs->current_value && node->gate.rhs->current_value;
                    } else {
                        node->current_value = false;
                    }
                    break;
                case(OR):
                    if (both) {
                        node->current_value = 
                            node->gate.lhs->current_value || node->gate.rhs->current_value;
                    } else {
                        node->current_value = false;
                    }
                    break;
                    
                case(NOT):
                    if (node->gate.lhs) {
                        node->current_value = !node->gate.lhs->current_value;
                    } else {
                        node->current_value = false;
                    }
                    break;
                };
                
                if (node->gate.lhs) {
                    node->gate.lhs->num_dependencies--;
                }
                if (node->gate.rhs) {
                    node->gate.rhs->num_dependencies--;
                }
                
            } break;
            case(THRUSTER): {
                if (node->thruster.input) {
                    if (node->thruster.input->current_value) {
                        node->current_value = true;
                        thrusters |= node->thruster.thruster;
                    } else {
                        node->current_value = false;
                    }
                    node->thruster.input->num_dependencies--;
                }
            } break;
            };
            node = node->next_in_q;
        }

    }
    return thrusters;
}
