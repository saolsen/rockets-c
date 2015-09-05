#ifndef _rockets_debug_h
#define _rockets_debug_h

// Rockets Debug Code

// Going to use nanovg for debug drawing.
#include "nanovg.h"
#include <stdint.h>

// Performance Debugging
typedef struct {
    int line_number;
    char* file_name;
    char* function_name;
    
    uint64_t clocks;
    uint64_t hit_count;
} DebugRecord;

DebugRecord debug_records[];
int num_debug_records;

// @TODO: Thread safety if we do multithreading.
// @NOTE: Internal functions, use the BEGIN_TIME_BLOCK and END_TIME_BLOCK macros below for
// instrumenting code.
int
_begin_timed_block(int index, int line_number, const char* file_name, const char* function_name)
{
    DebugRecord* record = debug_records + index;

    record->file_name = (char*)file_name;
    record->function_name = (char*)function_name;

    record->line_number = line_number;

    record->clocks -= performance_counter();
    record->hit_count++;
    return index;
}

void
_end_timed_block(int index)
{
    DebugRecord* record = debug_records + index;
    record->clocks += performance_counter();
}

/* @NOTE:
   These are the macros you use to instrument code for performance debugging.
   At the top of a block you want to time create a new time block.
   int block = BEGIN_TIME_BLOCK();

   Then at the end of the code you want to time end the time block.
   END_TIME_BLOCK(block);

   Timing information is stored in a table by __COUNTER__. This only works because we compile the
   whole game as one compilation unit. Some functions to print / visualize this timing information
   are below.
 */

#define BEGIN_TIME_BLOCK() _begin_timed_block(__COUNTER__, __LINE__, __FILE__, __FUNCTION__)
#define END_TIME_BLOCK(timer) _end_timed_block(timer)

// @TODO: I don't know what clocks should be set to to start the frame.
void
debug_setup_records()
{
    uint64_t start_tick = performance_counter();
    for (int i = 0; i < num_debug_records; i++) {
        DebugRecord* record = debug_records + i;
        record->clocks = start_tick;
        record->hit_count = 0;
    }
}

// @TODO: Replace with a scrollable thing in the scene.
// @TODO: Replace with a useful summary of the results of the timings.
// clocks is the total cycles per frame,

// @TODO: Things to calculate.
// - total time in function this frame.
// - number of times function was called this frame.
// - average time per function call (maybe outliers too?)

// @TODO: Add some graphs man! Graphs are dope!
void
debug_print_records()
{
    for (int i = 0; i < num_debug_records; i++) {
        DebugRecord* record = debug_records + i;
        log_info("Record %i, Clocks: %zi, Hits: %zi", i, record->clocks, record->hit_count);
    }
}

// Debug Drawing
// @NOTE: Debug drawing with nanovg is slightly different for node stuff vs space scene stuff.
// All of this is probably going to break once I start messing with resolutions of the rendering
// so I'll have to revisit.

// @TODO: Decide where you want this memory to come from. It's super nice to not have to pass
//        it in but feels dirty. Figure out what casey would do here.

// @TODO: Metadata, and maybe a better macro way to do this.
// Want a way to browse these things by their metadata and turn on / off debug drawings.
// need some UI integration. Like "show collisions" "show collision area for these two entities"
typedef enum {DebugObject_NAH,
              DebugObject_V2,
              DebugObject_BOX} DebugObjectType;

typedef enum {WHITE,
              BLUE,
              GREEN,
              RED} DebugColor;

typedef struct {
    DebugObjectType type;
    V2 origin;
    union {
        V2 size;
        V2 v;
    };
    DebugColor color;
} DebugObject;

typedef struct {
    int num_objects;
    DebugObject objects[512];
} DebugObjects;

DebugObjects _debug_objects;
// @TODO: Make these macros so when not in a debug build all of this can be turned off.

// Debug drawing functions for the space scene.
void
debug_draw_v2(V2 origin, V2 v, DebugColor color)
{
    DebugObject* obj = _debug_objects.objects + _debug_objects.num_objects++;
    obj->type = DebugObject_V2;
    obj->origin = origin;
    obj->v = v;
    obj->color = color;
}

void
debug_draw_line(float origin_x, float origin_y, float x, float y, DebugColor color)
{
    debug_draw_v2(v2(origin_x, origin_y), v2(x, y), color);
}

void
debug_draw_box(float origin_x, float origin_y, float width, float height, DebugColor color)
{
    DebugObject* obj = _debug_objects.objects + _debug_objects.num_objects++;
    obj->type = DebugObject_BOX;
    obj->origin = v2(origin_x, origin_y);
    obj->size = v2(width, height);
    obj->color = color;
}

// Call at the beginning of the scene.
void
debug_setup_drawing()
{
    for (int i = 0; i < ARRAY_COUNT(_debug_objects.objects); i++) {
        DebugObject* obj = _debug_objects.objects + i;
        obj->type = DebugObject_NAH;
    }
    _debug_objects.num_objects = 0;
}

// @NOTE: Should be called from within the rendering of the scene at the end but while
//        the drawing context is still translated. Handles the reverse Y drawing.
void
debug_draw_debug_drawings(NVGcontext* vg)
{   
    for (int i = 0; i < _debug_objects.num_objects; i++) {
        DebugObject obj = _debug_objects.objects[i];
        if (obj.type == DebugObject_NAH) continue;

        nvgBeginPath(vg);
        
        switch(obj.type) {
        case(DebugObject_NAH): break; //nah
        case(DebugObject_V2): {
            nvgMoveTo(vg, obj.origin.x, -obj.origin.y);
            nvgLineTo(vg, obj.origin.x + obj.v.x, -obj.origin.y - obj.v.y);
        } break;
        case(DebugObject_BOX): {
            nvgRect(vg,
                    obj.origin.x,
                    -obj.origin.y - obj.size.y,
                    obj.size.x,
                    obj.size.y);
        } break;
        }

        NVGcolor color;
        switch(obj.color) {
        case(WHITE):
            color = nvgRGBf(1, 1, 1);
            break;
        case(RED):
            color = nvgRGBf(1, 0, 0);
            break;
        case(GREEN):
            color = nvgRGBf(0, 1, 0);
            break;
        case(BLUE):
            color = nvgRGBf(0, 0, 1);
            break;
        }

        nvgStrokeColor(vg, color);
        nvgStroke(vg);
    }
}

#endif
