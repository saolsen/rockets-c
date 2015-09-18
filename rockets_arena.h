#ifndef _rockets_arena_h
#define _rockets_arena_h
/*
  This is an arena that I'm going to be using multiple places. It will be used to back 
  my gui command buffer to start but will probably also be used to back the nodestore and any other
  place I want to do memory stuff.

  I will have it work as a fixed buffer to start but should be able to create a resizable one too.

  Casey has a notion of a TempMemory which is basically a sub arena. He has an arena count on the
  main arena and he records when there's a new arena then pops the whole temp space when done.
  Memory management then works off overlapping stacks.
  - thread/process stack (like the memory for a worker process)
  - job stack (the memory for a certain process like generating a level)
  - function stack (the memory for a given function)

  Need to learn more about alignment and how to handle it.

  Renderer could use a pushbuffer too, then you can do the transform to screen coordinates there.

  casey allocates his pushbuffer from a transient state buffer, maybe I want that too.

  THIS IS ALL TODO
 */

typedef struct {
    uint8_t* base;
    size_t size;
    size_t used;
} MemoryArena;

void arena_initialize(MemoryArena* arena, void* memory, size_t size)
{
    arena->base = memory;
    arena->size = size;
    arena->used = 0;
}

// @TODO: Figure out how to handle alignment.
void* arena_push_size(MemoryArena* arena, size_t size)
{
    arena->used += size;
    assert(arena->used < arena->size);

    uint8_t* new_object = arena->base;
    arena->base += size;
    // @TODO: Handle alignment

    return new_object;
}

#define arena_push_struct(arena, type) (type*)arena_push_size(arena, sizeof(type))


/* // @TODO: Casey has alignment as a default arg = 4. I can't do that in c. */
/* void arena_initialize(MemoryArena* arena, void* memory, size_t size); */
/* uint8_t* arena_push_size(MemoryArena* arena, size_t size); */
/* #define PUSH_STRUCT(arena, type, ...) (type*)arena_push_size(arena, sizeof(type)) */

#endif
