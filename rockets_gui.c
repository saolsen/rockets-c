#include "rockets.h"

// Rockets GUI
void
gui_init(gg_Input input, float screen_width, float screen_height, float dt)
{
    // gui_state must be set to a valid GUIState before calling gui_init
    assert(current_gui_state);
    
    current_gui_state->input = input;
    current_gui_state->screen_width = screen_width;
    current_gui_state->screen_height = screen_height;
    current_gui_state->dt = dt;
}

void
gui_render(NVGcontext* vg)
{
    assert(current_gui_state);
    
}
