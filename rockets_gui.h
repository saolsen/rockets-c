#ifndef _rockets_gui_h
#define _rockets_gui_h

/*
  Going to try to keep the gui stuff better contained this time. A couple goals I have.
  1) Don't render until the end of the frame.

  Go look at IMGUI any time you get stuck. It seems like he did a very good job.
  Might even want to use that for any debug / level editor tools instead of doing it from
  scratch.

  I'm going to use nanovg for rendering since that's what I'm using but It could also be cool to
  have this work with opengl directly. I think the other imgui libraries generate a vertex buffer
  to be passed to a shader that draws the api.

  Try not to do *too* much of this until you have some real usage code.

  
  I don't know yet what to do about a node dragging panal. I have to decide where the dragging
  state gets stored. Maybe I just store the id of the node being dragged or something like that.
  Buttons are a pretty easy place to start.
 */
typedef struct {
    float x, y, w, h;
} GUIRect;

typedef enum {
    GUICommandType_GUICommandRect,
} GUICommandType;

typedef struct {
    GUICommandType type; // Command Type
    /* size_t next;         // Base pointer(byte) offset to next command. */
} GUICommandHeader;

typedef struct {
    GUIRect rect;
} GUICommandRect;

typedef struct {
    // Frame Information
    gg_Input input;
    float screen_height, screen_width, dt;
    
    // Command Buffer
    uint8_t* command_buffer_base;
    size_t command_buffer_size;
    size_t command_buffer_used;
    
} GUIState;

// Public API.
// Initialization.
typedef enum {GUI_ICON_SENSOR,
              GUI_ICON_PREDICATE,
              GUI_ICON_GATE,
              GUI_ICON_THRUSTER} GUI_ICON;

#endif
