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

    // @TODO: Draw the things in the command buffer.
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
