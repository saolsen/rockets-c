#ifndef _rockets_entitites_h
#define _rockets_entitites_h

// Entities
typedef enum {EntityType_NAH,
              EntityType_SHIP,
              EntityType_BOUNDRY,
              EntityType_GOAL} EntityType;

// @TODO: Probably need some info on animating this stuff, maybe that goes elsewhere.
typedef struct entity_ {
    int id;
    EntityType type;
    uint32_t flags;

    Position position;
    Position next_position;
    uint32_t thrusters;

    struct entity_* next_entity;
} Entity;

#endif
