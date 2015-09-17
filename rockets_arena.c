#include "rockets.h"

void arena_initialize(MemoryArena* arena, void* memory, size_t size)
{
    arena->base = memory;
    arena->size = size;
    arena->used = 0;
}

// @TODO: Figure out how to handle alignment.
uint8_t* arena_push_size(MemoryArena* arena, size_t size)
{
    arena->used += size;
    assert(arena->used < arena->size);

    uint8_t* new_object = arena->base;
    arena->base += size;
    // @TODO: Handle alignment

    return new_object;
}
