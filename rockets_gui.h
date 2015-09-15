#ifndef _rockets_gui_h
#define _rockets_gui_h
/*
  Going to try to keep the gui stuff better contained this time. A couple goals I have.
  1) Don't render until the end of the frame.
  2) Allow GUI functions to be called for anywhere.

  To accomplish these we'll have a global pointer to some GUI_MEMORY that is set at the beginning
  of the frame. We'll use that to store up any data we need as gui functions are called and then
  at the end of the frame call gui_render() which is the only function that gets passed a nanovg.
  Gotta make sure this pointer is set each update_and_render because it won't persist past lib
  reloads.

  Go look at IMGUI any time you get stuck. It seems like he did a very good job.
  Might even want to use that for any debug / level editor tools instead of doing it from
  scratch.

  To use.
  Each frame.
  Set current_gui_state to the current_gui_state
  Call gui_init, pass that frame's input and details. (maybe pass pointer here actually.
  Call any gui functions.
  At the end of the frame, call gui_render.
  
  Things are stored in GUIState accross frames so persist it accross frames.

  I'm going to use nanovg for rendering since that's what I'm using but It could also be cool to
  have this work with opengl directly. I think the other imgui libraries generate a vertex buffer
  to be passed to a shader that draws the api.

  Try not to do *too* much of this until you have some real usage code.

  --
  Gonna try to do this the way https://github.com/vurtun/zahnrad does it.
  I have a command_buffer that has all the different drawing commands to do for the frame.
  
  I don't know yet what to do about a node dragging panal. I have to decide where the dragging
  state gets stored. Maybe I just store the id of the node being dragged or something like that.
  Buttons are a pretty easy place to start.
 */
typedef struct {
    float x, y, w, h;
} GUIRect;

// Input checking helpers.
bool gui_rect_did_click(gg_Input input, GUIRect rect);

typedef enum {
    GUI_COMMAND_RECT
} GUICommandType;

typedef struct {
    GUICommandType type; // Command Type
    size_t next;         // Base pointer(byte) offset to next command.
} GUICommand;

typedef struct {
    GUICommand header;
    GUIRect rect;
} GUICommandRect;

typedef struct {
    // GUI Memory
    uint8_t memory[1024]; // @TODO: Could have this passed in to init.
    uint8_t* base;
    size_t size;
    size_t used;
} GUICommandBuffer;

void gui_command_buffer_init(GUICommandBuffer* command_buffer);
uint8_t* gui_command_buffer_push_object(GUICommandBuffer* command_buffer, size_t size);
#define GUI_COMMAND_PUSH(command_buffer, type)                          \
    (type*)gui_command_buffer_push_object(command_buffer, sizeof(type))
void gui_command_buffer_push_rect(GUICommandBuffer* command_buffer, GUIRect rect);

typedef struct {
    // Frame Information
    gg_Input input;
    float screen_height, screen_width, dt;

    // Command Buffer
    GUICommandBuffer command_buffer;
} GUIState;

GUIState* current_gui_state = NULL;

// Public API.
// Initialization.
void
gui_init(GUIState* gui_state, gg_Input input, float scree_height, float screen_width, float dt);
void
gui_render(NVGcontext* vg);

// GUI functions
bool gui_button_with_text(float x, float y, float width, float height, char* txt);

#endif
