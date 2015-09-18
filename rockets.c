#include "rockets.h"

#include "rockets_grid.c"
#include "rockets_sim.c"
#include "rockets_render.c"
#include "rockets_gui.c"
#include "rockets_nodes.c"

#define KILOBYTES(value) ((value)*1024LL)
#define MEGABYTES(value) (KILOBYTES(value)*1024LL)
const size_t gamestate_size = sizeof(GameState) + MEGABYTES(10);

static void*
game_setup(void* game_state, NVGcontext* vg)
{
    log_info("Setting up game");
    GameState* state = (GameState*)game_state;

    arena_initialize(&state->persistent_store,
                     (uint8_t*)game_state + sizeof(GameState),
                     gamestate_size - sizeof(GameState));

    state->node_store = nodestore_allocate(&state->persistent_store);
    state->gui_state = gui_allocate(&state->persistent_store, KILOBYTES(100));

    state->tick = 0;

    state->ship_position.tile = gridV(6, 2, -8);
    state->ship_position.facing = UP;


    // @TODO: If you have more than one font you need to store a
    // reference to this.
    int font = nvgCreateFont(vg,
                             "basic",
                             "SourceSansPro-Regular.ttf");
    // Assert that the font got loaded.
    assert(font >= 0);
    
    return state;
}

#define hexagon_grid_origin_x 650
#define hexagon_grid_origin_y 675

static void
game_update_and_render(void* gamestate,
                       NVGcontext* vg,
                       gg_Input input,
                       float dt)
{
    GameState* state = (GameState*)gamestate;

    // @TODO: Have an init_renderer call.
    current_vg = vg;
    gui_frame(state->gui_state, input, 1280, 720, dt);
    
    HexagonGrid grid = {.rows = 12,
                        .columns = 13,
                        .origin_x = hexagon_grid_origin_x,
                        .origin_y = hexagon_grid_origin_y,
                        .hexagon_size = 30};

    if (state->tick++ % 60 == 0) {
        state->ship_thrusters++;

        if (state->ship_thrusters == 32) {
            state->ship_thrusters = 0;
        }
    }

    draw_base_grid(grid);
    draw_hex_grid(grid);
    draw_ship(grid, state->ship_position, state->ship_thrusters, RED, YELLOW);

    Position next_position = next_ship_position(state->ship_position, state->ship_thrusters);
    V2 center = gridV_to_pixel(grid, next_position.tile);
    draw_grid_arrow(center, next_position.facing, WHITE);

    GridV mouse_over = pixel_to_gridV(grid, v2(input.mouse_x, input.mouse_y));
    draw_formatted_text(v2(hexagon_grid_origin_x+5,
                           hexagon_grid_origin_y+15), 24, WHITE,
                        "(%i,%i,%i)",
                        mouse_over.x, mouse_over.y, mouse_over.z);

    V2 pos = v2(35, 25);
    if (gui_button(state->gui_state, 10, 25, 20, 20, WHITE, GUI_ICON_SENSOR)) {
        // create new sensor node at pos;
        Node* new_node = nodestore_push_node(state->node_store, SENSOR);
        log_info("Create Sensor node");
        new_node->position = pos;
    }

    if (gui_button(state->gui_state, 10, 50, 20, 20, WHITE, GUI_ICON_PREDICATE)) {
        // create new predicate node at pos;
        log_info("Create Predicate");
        Node* new_node = nodestore_push_node(state->node_store, PREDICATE);
        new_node->position = pos;
    }

    if (gui_button(state->gui_state, 10, 75, 20, 20, WHITE, GUI_ICON_GATE)) {
        // create new gate node at pos;
        log_info("Create Gate node");
        Node* new_node = nodestore_push_node(state->node_store, GATE);
        new_node->position = pos;
    }

    if (gui_button(state->gui_state, 10, 100, 20, 20, WHITE, GUI_ICON_THRUSTER)) {
        // create new thruster node at pos;
        log_info("Create Thruster node");
        Node* new_node = nodestore_push_node(state->node_store, THRUSTER);
        new_node->position = pos;
    }

    gui_drag_panal_bounds(state->gui_state, 0, 0, hexagon_grid_origin_x, hexagon_grid_origin_y);
    for (int i = 0;
         i < state->node_store->node_buffer_used;
         i++) {
        Node* node = state->node_store->node_buffer_base + i;
        if (node->id == 0) continue;

        gui_dragable_rect(state->gui_state, &node->position, &node->id, 20, 20);

        // buttons


        // background

        
        // drag rect
        
    }
         
    

    /* if (gui_drag_from_panal(0, 0, 10, 10, ICON_SIGNAL)) { */
    /*     nodestore_push_node(&state->node_store, SENSOR); */
    /* } */

    /* draw_hex(grid, mouse_over, CYAN); */

    // show mouse
    draw_circle(v2(input.mouse_x, input.mouse_y), 3, GREEN);
    draw_formatted_text(v2(10,10), 12, WHITE,
                        "Mouse Position. (%i, %i)", input.mouse_x, input.mouse_y);

    gui_render(state->gui_state, vg);
}

const gg_Game gg_game_api = {
    .gamestate_size = gamestate_size,
    .init = game_setup,
    .update_and_render = game_update_and_render
};

/* DebugRecord debug_records[__COUNTER__]; */
/* int num_debug_records = ARRAY_COUNT(debug_records); */
