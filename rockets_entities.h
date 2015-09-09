#ifndef _rockets_entitites_h
#define _rockets_entitites_h

// Entities
typedef enum {EntityType_NAH,
              EntityType_SHIP,
              EntityType_BOUNDRY,
              EntityType_GOAL} EntityType;

typedef enum {EntityFlag_COLLIDES = (1 << 0)} EntityFlag;

// Start with a special case of just having all collision geometry be rectangles.
// For now going to have rotation be about the entitiy's position. Makes math easy
// and seems to make sense for things that are going to move around.

// @NOTE: This is basically the same as bounding box but the names are different because this is
// a different way to describe a rectangle.
typedef struct {
    union {
        struct {float x, y;};
        V2 offset;
    };
    union {
        struct {float width, height; };
        V2 size;
    };
} CollisionRect;


// How do I determine entity equality? Just pointer equality?
typedef struct entity_ {
    EntityType type;
    uint32_t flags;

    V2 position;
    int rotation;
    Thrusters thrusters;

    V2 velocity;

    // @TODO: I think I should probably use a memory area for this instead of allocating
    // 10 slots for every entity. Many will just have 1 piece and lots won't have any.
    // If I have a memory area I can track memory usage in it too.
    // Also could let me pack all collision pieces in a single array which might be dope
    // if I want to SIMD my collision checking.
    int num_collision_pieces;
    CollisionRect collision_pieces[10];

    struct entity_* next_entity;
} Entity;

#endif
