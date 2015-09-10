#ifndef _rockets_gui_h
#define _rockets_gui_h
// GUI stuff
// @TODO: rockets_gui

typedef struct {
    Node* node;
    BoundingBox bb;
    V2 draw_position;
    int input_index;
    Node* input_to;
} NodeBounds;

typedef enum { NE_NAH } NodeEventType;

typedef struct {
    NodeEventType type;
} NodeEvent;

typedef enum {GUI_NAH,
              GUI_DRAGGING_NODE,
              GUI_DRAGGING_INPUT,
              GUI_DRAGGING_OUTPUT,
              GUI_DRAGGING_SLIDER} GUI_State;

typedef enum {DT_NAH,
              DT_BODY,
              DT_OUTPUT,
              DT_INPUT,
              DT_SLIDER} DragTarget_Type;

typedef struct {
    DragTarget_Type type;
    int from_id;
    int from_input_num;
    V2 from_position; // can calculate from from_id but meh...
    V2 position;
    int value;
} DragTarget;

typedef struct {
    NVGcontext* vg;
    gg_Input input;
    GUI_State state;
    DragTarget drag_target;
} GUIState;

typedef enum { BS_NAH, BS_HOVER, BS_CLICK } ButtonState;
#endif
