#include "rockets.h"

#define GUICommandBufferPush(gui_state, type) (type*)gui_command_buffer_push_(gui_state, sizeof(type), GUICommandType_##type)
uint8_t*
gui_command_buffer_push_(GUIState* gui_state, size_t size, GUICommandType type)
{
    assert(gui_state->command_buffer_used + size <= gui_state->command_buffer_size);

    GUICommandHeader *header = (GUICommandHeader*)(gui_state->command_buffer_base +
                                                   gui_state->command_buffer_used);

    size += sizeof(*header);
    header->type = type;

    uint8_t* command_object = (uint8_t*)header + sizeof(*header);
    gui_state->command_buffer_used += size;

    return command_object;
}


GUICommandRect*
gui_command_buffer_push_rect(GUIState* gui_state, GUIRect rect, Color color, GUI_ICON icon)
{   
    GUICommandRect* command_rect = GUICommandBufferPush(gui_state, GUICommandRect);

    command_rect->rect = rect;
    command_rect->icon = icon;

    return command_rect;
}

// User Functions
GUIState*
gui_allocate(MemoryArena* arena, size_t size)
{
    GUIState* gui_state = arena_push_struct(arena, GUIState);
    gui_state->command_buffer_base = (uint8_t*)arena_push_size(arena, size);
    gui_state->command_buffer_size = size;
    gui_state->command_buffer_used = 0;

    gui_state->mouse_is_down = false;

    gui_state->dragging_id = NULL;
    
    return gui_state;
}

void
gui_frame(GUIState* gui_state, gg_Input input, float screen_width, float screen_height, float dt)
{
    gui_state->input = input;
    gui_state->screen_width = screen_width;
    gui_state->screen_height = screen_height;
    gui_state->dt = dt;

    if (input.mouse_down) {
        gui_state->mouse_is_down = true;
        gui_state->mouse_down_x = input.mouse_x;
        gui_state->mouse_down_y = input.mouse_y;
    }
}


void
gui_render(GUIState* gui_state, NVGcontext* vg)
{
    int num = 0;
    for (size_t offset = 0;
         offset != gui_state->command_buffer_used;) {
        GUICommandHeader* header = (GUICommandHeader*)(gui_state->command_buffer_base + offset);
        offset += sizeof(*header);

        uint8_t* command = (uint8_t*)header + sizeof(*header);

        switch(header->type) {
        case(GUICommandType_NOPE): {
            // Something going wrong bruh.
            assert(false);
        };
            
        case(GUICommandType_GUICommandRect): {
            GUICommandRect* command_rect  = (GUICommandRect*)command;
            GUIRect rect = command_rect->rect;

            nvgSave(vg);
            nvgFillColor(vg, nvgRGBf(1,1,1));
            nvgBeginPath(vg);
            nvgRect(vg, command_rect->rect.x, rect.y, rect.w, rect.h);
            nvgFill(vg);
            nvgRestore(vg);

            offset += sizeof(*command_rect);
            
        } break;
        };
        num++;
    }

    // @TODO: Record necessesary stuff for next frame.
    gui_state->command_buffer_used = 0;

    if (gui_state->input.mouse_up) {
        gui_state->mouse_is_down = false;
    }
}

// @TODO: Handle clicks better, longer clicks and drag off then back on etc.
bool
gui_rect_did_click(GUIState* gui_state, GUIRect rect)
{   
    return (gui_state->input.mouse_up &&
            // Mouse down was this frame
            (gui_state->input.mouse_down ||
            // Mouse down was on this button.
             (gui_state->mouse_down_x >= rect.x &&
              gui_state->mouse_down_x <= rect.x + rect.w &&
              gui_state->mouse_down_y >= rect.y &&
              gui_state->mouse_down_y <= rect.y + rect.h)) &&
            // Mouse up was on this button.
            (gui_state->input.mouse_x >= 
             gui_state->input.mouse_x <= rect.x + rect.w &&
             gui_state->input.mouse_y >= rect.y &&
             gui_state->input.mouse_y <= rect.y + rect.h));
}


bool
gui_button(GUIState* gui_state, float x, float y, float width, float height,
           Color color, GUI_ICON icon)
{
    GUIRect rect = {.x = x, .y = y, .w = width, .h = height};

    gui_command_buffer_push_rect(gui_state, rect, color, icon);

    // Check for click.
    return gui_rect_did_click(gui_state, rect);
}


void
gui_drag_panal_bounds(GUIState* gui_state, float x, float y, float width, float height)
{
    gui_state->drag_panal_rect = (GUIRect){.x = x, .y = y, .w = width, .h = height};
}


// Modifies pos when it gets dragged.
void
gui_dragable_rect(GUIState* gui_state, V2* pos, void* id, float width, float height)
{
    // @TODO: make sure it's in the bounds.
    if (gui_state->input.start_dragging) {
        log_info("Start Dragging");

        if (gui_state->input.mouse_x >= pos->x &&
            gui_state->input.mouse_x <= pos->x + width &&
            gui_state->input.mouse_y >= pos->y &&
            gui_state->input.mouse_y <= pos->y + height) {
            gui_state->dragging_id = id;
            // @TODO: Do I not need this?
            gui_state->dragging_rect =
                (GUIRect){.x = pos->x, .y = pos->y, .w = width, .h = height};
            log_info("Start Dragging");
        }

    } else if (gui_state->input.end_dragging) {
        gui_state->dragging_id = NULL;
        log_info("End Dragging");

    } else if (gui_state->input.is_dragging &&
               gui_state->input.mouse_motion &&
               gui_state->dragging_id == id) {
        pos->x += gui_state->input.mouse_xrel;
        pos->y += gui_state->input.mouse_yrel;
        log_info("Is Dragging");
    }

    GUIRect rect = (GUIRect){.x = pos->x, .y = pos->y, .w = width, .h = height};
    gui_command_buffer_push_rect(gui_state, rect, BLUE, GUI_ICON_NONE);
}

// pass an id, store id and last dragable rect bounds, and currently dragging rect

// What of this is rendering and what of this is gui?
// rectangle
// text

// Dragable rect.


/*
  One thing I really need to figure out for imgui stuff is what a good interface to work with
  dragable stuff is going to be.
 */

// Position dragged to.

// Begin drag pane.
// Begin a node.
// The void* id thing is clever.

// A button that returns true when you click it and drag off. Used as a panel for creating nodes.
// pos will be filled with the position that the mouse is at when it's dragged off.

// @TODO: Need to store some *dragging from* information in the guistate to match this later.
/* bool */
/* gui_drag_off_button(GUIState* gui_state, V2* out_pos, float x, float y, float width, float height, GUI_ICON icon) */
/* { */
/*     GUIRect rect = {.x = x, .y = y, .w = width, .h = height}; */

/*     gui_command_buffer_push_rect(gui_state, rect); */

/*     // If dragging and dragging started from in this rect and no other hot dragging object */
/*     // and dragged to an area outside this button then return true; */
    
/*     return false; */
/* } */
