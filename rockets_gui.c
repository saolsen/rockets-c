#include "rockets.h"

// Helper Functions
bool
gui_rect_did_click(gg_Input input, GUIRect rect)
{
    return (input.click &&
            input.mouse_x >= rect.x &&
            input.mouse_x <= rect.x + rect.w &&
            input.mouse_y >= rect.y &&
            input.mouse_y <= rect.y + rect.h);
}

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
gui_command_buffer_push_rect(GUIState* gui_state, GUIRect rect)
{   
    GUICommandRect* command_rect = GUICommandBufferPush(gui_state, GUICommandRect);

    command_rect->rect = rect;

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
    return gui_state;
}

void
gui_frame(GUIState* gui_state, gg_Input input, float screen_width, float screen_height, float dt)
{
    gui_state->input = input;
    gui_state->screen_width = screen_width;
    gui_state->screen_height = screen_height;
    gui_state->dt = dt;
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
}


// @TODO: Enable an icon instead of text.
bool
gui_button_with_text(GUIState* gui_state, float x, float y, float width, float height, char* txt)
{
    GUIRect rect = {.x = x, .y = y, .w = width, .h = height};

    // Push rect for drawing.
    gui_command_buffer_push_rect(gui_state, rect);

    // Check for click.
    return gui_rect_did_click(gui_state->input, rect);
    
    return true;
}

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
bool
gui_drag_off_button(GUIState* gui_state, V2* out_pos, float x, float y, float width, float height, GUI_ICON icon)
{
    GUIRect rect = {.x = x, .y = y, .w = width, .h = height};

    gui_command_buffer_push_rect(gui_state, rect);

    // If dragging and dragging started from in this rect and no other hot dragging object
    // and dragged to an area outside this button then return true;
    
    return false;
}
