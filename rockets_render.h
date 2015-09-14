#ifndef _rockets_render_h
#define _rockets_render_h

/*
  This works similar to how the gui namespace works. There is a pointer that is used internally
  by the rendering functions.
  All the drawing happens when the render functions are called though, unlike how gui renders
  everything at the end.
 */

NVGcontext* current_vg;

typedef enum {WHITE, RED, GREEN, BLUE, YELLOW, CYAN, MAGENTA, BLACK} Color;

#endif
