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


// @TODO: This could pull from an arena instead of allocating itself.
void
gui_command_buffer_init(GUICommandBuffer* command_buffer)
{
    command_buffer->base = command_buffer->memory;
    command_buffer->size = ARRAY_COUNT(command_buffer->memory);
    command_buffer->used = 0;
}


uint8_t*
gui_command_buffer_push_object(GUICommandBuffer* command_buffer, size_t size)
{
    uint8_t* command_object = command_buffer->base + command_buffer->used;
    command_buffer->used += size;
    // @TODO: Add correct alignment offset here.
    return command_object;
}


void
gui_command_buffer_push_rect(GUICommandBuffer* command_buffer, GUIRect rect)
{
    GUICommandRect* command_rect = GUI_COMMAND_PUSH(command_buffer, GUICommandRect);

    command_rect->header.type = GUI_COMMAND_RECT;
    command_rect->header.next = command_buffer->used;

    command_rect->rect = rect;
}


// User Functions
void
gui_init(GUIState* gui_state, gg_Input input, float screen_width, float screen_height, float dt)
{
    assert(gui_state);
    current_gui_state = gui_state;

    gui_command_buffer_init(&gui_state->command_buffer);
    current_gui_state->input = input;
    current_gui_state->screen_width = screen_width;
    current_gui_state->screen_height = screen_height;
    current_gui_state->dt = dt;
}


void
gui_render(NVGcontext* vg)
{
    assert(current_gui_state);
    
    for (int i = 0; i != current_gui_state->command_buffer.used;) {
        GUICommand* command = (GUICommand*)current_gui_state->command_buffer.base + i;

        switch(command->type) {
        case(GUI_COMMAND_RECT): {
            GUIRect rect = ((GUICommandRect*)command)->rect;

            nvgSave(vg);
            nvgFillColor(vg, nvgRGBf(1,1,1));
            nvgBeginPath(vg);
            nvgRect(vg, rect.x, rect.y, rect.w, rect.h);
            nvgFill(vg);
            nvgRestore(vg);
            
        } break;
        };

        i = command->next;
    }
}


// @TODO: Enable an icon instead of text.
bool
gui_button_with_text(float x, float y, float width, float height, char* txt)
{
    assert(current_gui_state);
    GUIRect rect = {.x = x, .y = y, .w = width, .h = height};

    // Push rect for drawing.
    gui_command_buffer_push_rect(&current_gui_state->command_buffer, rect);

    // Check for click.
    return gui_rect_did_click(current_gui_state->input, rect);
    
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
gui_drag_off_button(V2* out_pos, float x, float y, float width, float height, GUI_ICON icon)
{
    assert(current_gui_state);
    GUIRect rect = {.x = x, .y = y, .w = width, .h = height};

    gui_command_buffer_push_rect(&current_gui_state->command_buffer, rect);

    // If dragging and dragging started from in this rect and no other hot dragging object
    // and dragged to an area outside this button then return true;
    
    return false;
}
