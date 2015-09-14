#ifndef _rockets_gui_h
#define _rockets_gui_h
/*
  Going to try to keep the gui stuff better contained this time. A couple goals I have.
  1) Don't render until the end of the frame.
  2) Allow GUI functions to be called for anywhere.

  To accomplish these we'll have a global pointer to some GUI_MEMORY that is set at the beginning
  of the frame. We'll use that to store up any data we need as gui functions are called and then
  at the end of the frame call gui_render() which is the only function that gets passed a nanovg.

  Go look at IMGUI any time you get stuck. It seems like he did a very good job.
  Might even want to use that for any debug / level editor tools instead of doing it from scratch.

  To use.
  During initialization, set gui_state to a valid GUIState.
  Each frame.
  Call gui_init, pass that frame's input and details.
  Call any gui functions.
  At the end of the frame, call gui_render.
  
  Things are stored in GUIState accross frames so persist it accross frames.
 */

typedef struct {
    // Frame Information
    gg_Input input;
    float screen_height, screen_width, dt;

} GUIState;

GUIState* current_gui_state = NULL;

// Beginning and end of frame.
void gui_init(gg_Input input, float scree_height, float screen_width, float dt);
void gui_render(NVGcontext* vg);

// GUI functions


#endif
