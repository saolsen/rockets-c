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
    command_rect->color = color;

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

    gui_state->drag_state = DRAGGING_NOTHING;
    gui_state->dragging_id = NULL;
    gui_state->dragging_avoid_id = NULL;

    gui_state->dragging_source_group = -1;

    gui_state->drag_target_result_for = NULL;
    gui_state->drag_target_result_id = NULL;

    gui_state->drag_target_buffer_base =
        (GUIDragTarget*)arena_push_size(arena, sizeof(GUIDragTarget) * 128);
    gui_state->drag_target_buffer_size = 128;
    gui_state->drag_target_buffer_used = 0;
    
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
            nvgStrokeColor(vg, get_color(command_rect->color));
            nvgBeginPath(vg);
            nvgRect(vg, command_rect->rect.x, rect.y, rect.w, rect.h);
            nvgStroke(vg);
            nvgRestore(vg);

            offset += sizeof(*command_rect);
            
        } break;
        };
        num++;
    }

    if (gui_state->input.end_dragging) {

        switch(gui_state->drag_state) {
        case(DRAGGING_NOTHING): {
            // nothing to do
        } break;
        case(DRAGGING_PANAL): {
            gui_state->drag_state = DRAGGING_NOTHING;
            gui_state->dragging_id = NULL;
        } break;
        case(DRAGGING_SOURCE): {

            for (int i = 0; i < gui_state->drag_target_buffer_used; i++) {
                GUIDragTarget* drag_target = gui_state->drag_target_buffer_base + i;
                
                if (drag_target->id != gui_state->dragging_avoid_id &&
                    drag_target->drag_target_group == gui_state->dragging_source_group &&
                    gui_state->input.mouse_x >= drag_target->rect.x &&
                    gui_state->input.mouse_x <= drag_target->rect.x + drag_target->rect.w &&
                    gui_state->input.mouse_y >= drag_target->rect.y &&
                    gui_state->input.mouse_y <= drag_target->rect.y + drag_target->rect.h) {

                    gui_state->drag_target_result_for = gui_state->dragging_id;
                    gui_state->drag_target_result_id = drag_target->id;
                    
                    break;
                }
            }
            
        } break;
        };

        gui_state->drag_state = DRAGGING_NOTHING;
    }

    gui_state->command_buffer_used = 0;
    gui_state->drag_target_buffer_used = 0;
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
    gui_state->drag_panal_bounds = (GUIRect){.x = x, .y = y, .w = width, .h = height};
}


// Modifies pos when it gets dragged.
void
gui_dragable_rect(GUIState* gui_state, V2* pos, void* id, float width, float height)
{
    // @TODO: make sure it's in the bounds.
    if (gui_state->input.start_dragging &&
        gui_state->drag_state == DRAGGING_NOTHING) {

        // Check is mouse is over panal.
        if (gui_state->input.mouse_x >= pos->x &&
            gui_state->input.mouse_x <= pos->x + width &&
            gui_state->input.mouse_y >= pos->y &&
            gui_state->input.mouse_y <= pos->y + height) {

            gui_state->dragging_id = id;
            gui_state->drag_state = DRAGGING_PANAL;
        }

    } else if (gui_state->input.is_dragging &&
               gui_state->drag_state == DRAGGING_PANAL &&
               gui_state->dragging_id == id &&
               gui_state->input.mouse_motion) {
        
        pos->x += gui_state->input.mouse_xrel;
        pos->y += gui_state->input.mouse_yrel;

    }

    GUIRect rect = (GUIRect){.x = pos->x, .y = pos->y, .w = width, .h = height};
    gui_command_buffer_push_rect(gui_state, rect, BLUE, GUI_ICON_NONE);
}


// Returns true if the mouse is currently over the rect.
bool
gui_mouseover_rect(GUIState* gui_state, V2 pos, float width, float height)
{
    return (gui_state->input.mouse_x >= pos.x &&
            gui_state->input.mouse_x <= pos.x + width &&
            gui_state->input.mouse_y >= pos.y &&
            gui_state->input.mouse_y <= pos.y + height);
}


void
gui_drag_target(GUIState* gui_state, void* id, int drag_target_group,
                float x, float y, float width, float height)
{
    // Record all the drag targets, (these are gonna have to persist a frame.)
    assert(gui_state->drag_target_buffer_used < gui_state->drag_target_buffer_size);
    GUIDragTarget* drag_target =
        gui_state->drag_target_buffer_base + gui_state->drag_target_buffer_used++;

    drag_target->rect = (GUIRect){.x = x, .y = y, .w = width, .h = height};
    drag_target->drag_target_group = drag_target_group;
    drag_target->id = id;
}

// Returns the id of the drag target it was dragged into. Has 1 frame of lag.
// @NOTE: To avoid one frame of lag could poll for the result after recording all
//        drag targets.
void*
gui_drag_source(GUIState* gui_state, void* id, void* avoid_id, int drag_target_group, float x, float y, float width, float height)
{
    GUIRect rect = (GUIRect){.x = x, .y = y, .w = width, .h = height};
    gui_command_buffer_push_rect(gui_state, rect, CYAN, GUI_ICON_NONE);

    if (gui_state->drag_target_result_for == id) {
        void* result = gui_state->drag_target_result_id;
        gui_state->drag_target_result_id = NULL;
        gui_state->drag_target_result_for = NULL;
        return result;
    }
    
    // Figure out if we start dragging here.
    if (gui_state->input.start_dragging &&
        gui_state->drag_state != DRAGGING_SOURCE) {
        if (gui_state->input.mouse_x >= x &&
            gui_state->input.mouse_x <= x + width &&
            gui_state->input.mouse_y >= y &&
            gui_state->input.mouse_y <= y + height) {

            gui_state->dragging_id = id;
            gui_state->dragging_avoid_id = avoid_id;
            gui_state->dragging_source_group = drag_target_group;
            gui_state->drag_state = DRAGGING_SOURCE;
        }
    }

    // @TODO: Push arrow drawing stuff.
    // @TODO: Need to figure out how the dragging arrow is drawn.
    return NULL;
}
