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

  This really needs cleaning up already. I need to decide when I'm using xywh, poswh or rects.
 */
typedef enum {GUI_ICON_NONE,
              GUI_ICON_SENSOR,
              GUI_ICON_PREDICATE,
              GUI_ICON_GATE,
              GUI_ICON_THRUSTER,
              GUI_ICON_DESTROY} GUI_ICON;

typedef struct {
    float x, y, w, h;
} GUIRect;

typedef enum {
    GUICommandType_NOPE,
    GUICommandType_GUICommandRect,
} GUICommandType;

typedef struct {
    GUICommandType type; // Command Type
} GUICommandHeader;

typedef struct {
    GUI_ICON icon;
    GUIRect rect;
    Color color;
} GUICommandRect;

typedef struct {
    GUIRect rect;
    int drag_target_group;
    void* id;
} GUIDragTarget;

typedef enum {DRAGGING_NOTHING,
              DRAGGING_PANAL,
              DRAGGING_SOURCE} DragState;

typedef struct {
    // Frame Information
    gg_Input input;
    float screen_height, screen_width, dt;
    
    // Command Buffer
    uint8_t* command_buffer_base;
    size_t command_buffer_size;
    size_t command_buffer_used;

    // State (a lot of this could just go in input if I thought it out better)
    float mouse_down_x;
    float mouse_down_y;
    bool mouse_is_down;

    // Dragging stuff.
    GUIRect drag_panal_bounds;
    
    DragState drag_state;
    void* dragging_id;
    // @TODO: Kind of hacky.
    void* dragging_avoid_id;

    int dragging_source_group;

    void* drag_target_result_for;
    void* drag_target_result_id;

    // Buffer for dragging and drag targets. Is this a good move?
    // One big buffer to start, not efficient at all.
    GUIDragTarget* drag_target_buffer_base;
    int drag_target_buffer_size;
    int drag_target_buffer_used;
    
} GUIState;

#endif
