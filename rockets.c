#include "rockets.h"

#include "rockets_grid.c"
#include "rockets_sim.c"
#include "rockets_render.c"
#include "rockets_gui.c"
#include "rockets_nodes.c"
#include "rockets_entities.c"

#define KILOBYTES(value) ((value)*1024LL)
#define MEGABYTES(value) (KILOBYTES(value)*1024LL)
const size_t gamestate_size = sizeof(GameState) + MEGABYTES(10);

void
load_level(GameState* state, int level)
{   
    state->level_status = PAUSED;
    state->current_level = level;

    state->running_time = 0;
    state->next_tick = 0;

    // Reset entities
    state->last_used_entity_id = 0;
    state->num_entities = 0;
    state->first_free_entity = NULL;

    // @HARDCODE: Load some entities for level 1.
    Entity* ship = push_entity(state, EntityType_SHIP);
    ship->position.tile = gridV(6, -2, -4);
    ship->next_position = ship->position;
    state->ship_entity = ship;
    
    Entity* goal = push_entity(state, EntityType_GOAL);
    goal->position.tile = gridV(6, 8, -14);
    goal->next_position = goal->position;
    state->goal_entity = goal;
}


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

    state->running_time = 0;
    state->next_tick = 0;

    state->level_status = PAUSED;
    state->current_level = 0;

    state->last_used_entity_id = 0;
    state->entities = arena_push_size(&state->persistent_store, sizeof(Entity) * 128);
    state->num_entities = 0;
    state->max_entities = 128;
    state->first_free_entity = NULL;

    state->ship_entity = NULL;
    state->goal_entity = NULL;
    // Must set ship and goal entity.
    load_level(state, 1);
    
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

    GridV mouse_over = pixel_to_gridV(grid, v2(input.mouse_x, input.mouse_y));
    draw_formatted_text(v2(hexagon_grid_origin_x+5,
                           hexagon_grid_origin_y+15), 24, WHITE,
                        "(%i,%i,%i)",
                        mouse_over.x, mouse_over.y, mouse_over.z);

    // GUI

    V2 pos = v2(35, 25);
    if (gui_button(state->gui_state, 10, 25, 20, 20, WHITE, GUI_ICON_SENSOR)) {
        // create new sensor node at pos;
        Node* new_node = nodestore_push_node(state->node_store, SENSOR);
        log_info("Create Sensor node");
        new_node->position = pos;
    }

    if (gui_button(state->gui_state, 10, 50, 20, 20, WHITE, GUI_ICON_PREDICATE)) {
        // create new predicate node at pos;
        log_info("Create Constant");
        Node* new_node = nodestore_push_node(state->node_store, CONSTANT);
        new_node->position = pos;
    }

    if (gui_button(state->gui_state, 10, 75, 20, 20, WHITE, GUI_ICON_PREDICATE)) {
        // create new predicate node at pos;
        log_info("Create Predicate");
        Node* new_node = nodestore_push_node(state->node_store, PREDICATE);
        new_node->position = pos;
    }

    if (gui_button(state->gui_state, 10, 100, 20, 20, WHITE, GUI_ICON_GATE)) {
        // create new gate node at pos;
        log_info("Create Gate node");
        Node* new_node = nodestore_push_node(state->node_store, GATE);
        new_node->position = pos;
    }

    if (gui_button(state->gui_state, 10, 125, 20, 20, WHITE, GUI_ICON_THRUSTER)) {
        // create new thruster node at pos;
        log_info("Create Thruster node");
        Node* new_node = nodestore_push_node(state->node_store, THRUSTER);
        new_node->position = pos;
    }

    gui_drag_panal_bounds(state->gui_state, 0, 0, hexagon_grid_origin_x, hexagon_grid_origin_y);

    Node* node_mouse_is_over = NULL;

    int numeric_node = 1;
    int boolean_node = 2;

    TileSet hilight_region = TILESET_EMPTY;
    bool show_hilight_region = false;
    
    for (int i=0; i<state->node_store->node_buffer_used; i++) {
        Node* node = state->node_store->node_buffer_base + i;
        if (node->id == 0) continue;

        // Make node dragable.
        gui_dragable_rect(state->gui_state, &node->position, &node->id, 100, 60);

        // Check if mouse is over Node.
        bool mouse_over_node = gui_mouseover_rect(state->gui_state, node->position, 100, 60);
        if (mouse_over_node) {
            node_mouse_is_over = node;
        }

        // Delete Button
        if (gui_button(state->gui_state,
                       node->position.x, node->position.y,
                       10, 10, RED, GUI_ICON_DESTROY)) {
            nodestore_delete_node(state->node_store, node->id);
        }

        // @DEBUG: Current Value
        draw_formatted_text(v2_minus(node->position, v2(0, 10)),
                            18, GREEN, "%i", node->current_value);
        

        switch(node->type) {
        case(SENSOR): {
            // Draw a little grid, have it be a button.
            // On click, set which sensor it is.
            HexagonGrid sensor_grid = {.rows = 1,
                                       .columns = 1,
                                       .origin_x = node->position.x + 18,
                                       .origin_y = node->position.y + 40,
                                       .hexagon_size = 8};

            GridV directional_tiles[] = {
                gridV(0,1,-1), // UP
                gridV(-1,1,0), // LEFT_UP
                gridV(-1,0,1), // LEFT_DOWN
                gridV(0,-1,1), // DOWN
                gridV(1,-1,0), // RIGHT_DOWN
                gridV(1,0,-1), // RIGHT_UP
            };
            
            // @DEBUG
            /* draw_hex_grid(grid); */

            // Mouse over
            GridV mouse_over = pixel_to_gridV(sensor_grid, v2(input.mouse_x, input.mouse_y));
            bool select_on_grid = false;
            Direction hilighted_direction;
            for (int i=0; i<ARRAY_COUNT(directional_tiles); i++) {
                draw_hex(sensor_grid, directional_tiles[i], WHITE, true);
                
                if (i != node->sensor.sensor_direction &&
                    directional_tiles[i].x == mouse_over.x &&
                    directional_tiles[i].y == mouse_over.y &&
                    directional_tiles[i].z == mouse_over.z) {
                    select_on_grid = true;

                    // button here
                    hilighted_direction = i;
                    if (gui_button(state->gui_state,
                                   node->position.x + 5, node->position.y + 11,
                                   42, 44, BLACK, GUI_ICON_NONE)) {
                        node->sensor.sensor_direction = i;
                    }
                }
            }

            // Draw current direction.
            draw_hex(sensor_grid, directional_tiles[node->sensor.sensor_direction], YELLOW, true);

            // Draw possible selected direction.
            if (select_on_grid) {
                draw_hex(sensor_grid, mouse_over, MAGENTA, true);
                // Show region this would pertain to.
                hilight_region = tileset_above(grid,
                                               state->ship_entity->position.tile,
                                               rotate_direction(state->ship_entity->position.facing,
                                                                hilighted_direction));
                show_hilight_region = true;
            } else if (mouse_over_node) {
                // Show region this node pertains to.
                hilight_region = tileset_above(grid,
                                               state->ship_entity->position.tile,
                                               rotate_direction(state->ship_entity->position.facing,
                                                                node->sensor.sensor_direction));
                show_hilight_region = true;
            }

            // Ship, center.
            draw_hex(sensor_grid, gridV(0,0,0), RED, true);

            if (gui_button(state->gui_state,
                           node->position.x+85, node->position.y+30,
                           10, 10, MAGENTA, GUI_ICON_DESTROY)) {
                //@TODO: Toggle which entity this node is about.
               
            }

            gui_drag_target(state->gui_state, node, numeric_node,
                            node->position.x, node->position.y, 100, 60);

            // On hover, show on the main map what tiles it's looking at and what number they are.

            // On hover also show the other entities in the scene and where they are.

            // Have an entity selector, a sensor node is specific to an entity?
            
        } break;

        case(PREDICATE): {
            if (gui_button(state->gui_state,
                           node->position.x+47.5, node->position.y+45,
                           10, 10, MAGENTA, GUI_ICON_DESTROY)) {
                node->predicate.predicate = ((int)node->predicate.predicate+1) % 6;
            }

            // @TODO: Center the text, this looks really bad.
            draw_formatted_text(v2(node->position.x + 43,
                                   node->position.y+ 35),
                                24, WHITE, predicate_names[node->predicate.predicate]);

            gui_drag_target(state->gui_state, node, boolean_node,
                            node->position.x, node->position.y, 100, 60);

            if (node->predicate.lhs) {
                // draw it
                draw_line(v2_plus(node->position, v2(25,0)),
                          v2_plus(node->predicate.lhs->position, v2(50,60)),
                          CYAN);

                if (gui_button(state->gui_state,
                              node->position.x+5, node->position.y+20,
                              10, 10, RED, GUI_ICON_DESTROY)) {
                    node->predicate.lhs = NULL;
                }
                
            } else {
                Node* left_connect_to_node = gui_drag_source(state->gui_state,
                                                             &node->predicate.lhs,
                                                             node,
                                                             numeric_node,
                                                             node->position.x + 10,
                                                             node->position.y + 20,
                                                             25,
                                                             25);
                if (left_connect_to_node) {
                    node->predicate.lhs = left_connect_to_node;
                }
            }

            if (node->predicate.rhs) {
                // draw it
                draw_line(v2_plus(node->position, v2(75,0)),
                          v2_plus(node->predicate.rhs->position, v2(50,60)),
                          CYAN);

                if (gui_button(state->gui_state,
                              node->position.x+85, node->position.y+25,
                              10, 10, RED, GUI_ICON_DESTROY)) {
                    node->predicate.rhs = NULL;
                }
                
                
            } else {
                Node* right_connect_to_node = gui_drag_source(state->gui_state,
                                                              &node->predicate.rhs,
                                                              node,
                                                              numeric_node,
                                                              node->position.x + 70,
                                                              node->position.y + 20,
                                                              25,
                                                              25);
                if (right_connect_to_node) {
                    node->predicate.rhs = right_connect_to_node;
                }
            }

            // Show the current values on the sides of the predicate.

        } break;
            
        case(CONSTANT): {
            if (gui_button(state->gui_state,
                           node->position.x+85, node->position.y+15,
                           10, 10, MAGENTA, GUI_ICON_DESTROY)) {
                node->constant.value++;
            }

            if (gui_button(state->gui_state,
                           node->position.x+85, node->position.y+35,
                           10, 10, MAGENTA, GUI_ICON_DESTROY)) {
                node->constant.value--;
            }
            
            draw_formatted_text(v2(node->position.x + 43, node->position.y+ 35), 24, WHITE,
                                "%i", node->constant.value);
            
            gui_drag_target(state->gui_state, node, numeric_node,
                            node->position.x, node->position.y, 100, 60);
        } break;

        case(GATE): {
            if (gui_button(state->gui_state,
                           node->position.x+47.5, node->position.y+45,
                           10, 10, MAGENTA, GUI_ICON_DESTROY)) {
                node->gate.gate = ((int)node->gate.gate+1) % 3;
            }

            // @TODO: Center the text, this looks really bad.
            draw_formatted_text(v2(node->position.x + 37, node->position.y+ 35), 20, WHITE,
                gate_names[node->gate.gate]);

            gui_drag_target(state->gui_state, node, boolean_node,
                            node->position.x, node->position.y, 100, 60);

            if (node->gate.lhs) {
                // draw it
                draw_line(v2_plus(node->position, v2(25,0)),
                          v2_plus(node->gate.lhs->position, v2(50,60)),
                          CYAN);

                if (gui_button(state->gui_state,
                              node->position.x+5, node->position.y+20,
                              10, 10, RED, GUI_ICON_DESTROY)) {
                    node->gate.lhs = NULL;
                }
                
            } else {
                Node* left_connect_to_node = gui_drag_source(state->gui_state,
                                                             &node->gate.lhs,
                                                             node,
                                                             boolean_node,
                                                             node->position.x + 10,
                                                             node->position.y + 20,
                                                             25,
                                                             25);
                if (left_connect_to_node) {
                    node->gate.lhs = left_connect_to_node;
                }
            }

            if (node->gate.gate != NOT) { 
                if (node->gate.rhs) {
                    // draw it
                    draw_line(v2_plus(node->position, v2(75,0)),
                              v2_plus(node->gate.rhs->position, v2(50,60)),
                              CYAN);

                    if (gui_button(state->gui_state,
                                   node->position.x+85, node->position.y+25,
                                   10, 10, RED, GUI_ICON_DESTROY)) {
                        node->gate.rhs = NULL;
                    }
                
                
                } else {
                    Node* right_connect_to_node = gui_drag_source(state->gui_state,
                                                                  &node->gate.rhs,
                                                                  node,
                                                                  boolean_node,
                                                                  node->position.x + 70,
                                                                  node->position.y + 20,
                                                                  25,
                                                                  25);
                    if (right_connect_to_node) {
                        node->gate.rhs = right_connect_to_node;
                    }
                }
            }
        } break;

        case(THRUSTER): {
            if (gui_button(state->gui_state,
                           node->position.x+47.5, node->position.y+45,
                           10, 10, MAGENTA, GUI_ICON_DESTROY)) {
                node->gate.gate = ((int)node->thruster.thruster << 1) % 32;
                if (node->gate.gate == 0) node->gate.gate++;
            }

            // @TODO: This means nothing, draw a rocket with selectors.
            draw_formatted_text(v2(node->position.x + 37, node->position.y+ 35), 20, WHITE,
                                "%i", node->thruster.thruster);

            if (node->thruster.input) {
                // draw it
                draw_line(v2_plus(node->position, v2(50,0)),
                          v2_plus(node->thruster.input->position, v2(50,60)),
                          CYAN);

                if (gui_button(state->gui_state,
                               node->position.x+45, node->position.y+5,
                               10, 10, RED, GUI_ICON_DESTROY)) {
                    node->thruster.input = NULL;
                }
                
                
            } else {
                Node* connect_to_node = gui_drag_source(state->gui_state,
                                                        &node->thruster.input,
                                                        node,
                                                        boolean_node,
                                                        node->position.x + 70,
                                                        node->position.y + 20,
                                                        25,
                                                        25);
                if (connect_to_node) {
                    node->thruster.input = connect_to_node;
                }
            }
        } break;
            
        }   
    }
    draw_base_grid(grid);

    if (show_hilight_region) {
        // Region Inequalities
        // 0 UP         y >= z
        // 1 LEFT_UP    y >= x
        // 2 LEFT_DOWN  z >= x
        // 3 DOWN       z >= y
        // 4 RIGHT_DOWN x >= y
        // 5 RIGHT_UP   x >= z

        GridV ship_pos = state->ship_entity->position.tile;

        // @TODO: Calculate the bounds of the grid.
        for (int x=ship_pos.x-20; x<=ship_pos.x+20; x++) {
            for (int y=ship_pos.y-20; y<=ship_pos.y+20; y++) {
                for (int z=ship_pos.z-20; z<=ship_pos.z+20; z++) {
                    GridV tile = gridV(x,y,z);
                    if (x + y + z == 0 && point_on_grid(grid, tile)) {

                        GridV check_point = gridV_minus(tile,
                                                        ship_pos);

                        // @OPTOMIZE, compute this part before looping.
                        bool in_region = false;
                        switch(hilight_region.direction) {
                        case(UP):
                            in_region = check_point.y >= check_point.z;
                            break;
                        case(LEFT_UP):
                            in_region = check_point.y >= check_point.x;
                            break;
                        case(LEFT_DOWN):
                            in_region = check_point.z >= check_point.x;
                            break;
                        case(DOWN):
                            in_region = check_point.z >= check_point.y;
                            break;
                        case(RIGHT_DOWN):
                            in_region = check_point.x >= check_point.y;
                            break;
                        case(RIGHT_UP):
                            in_region = check_point.x >= check_point.z;
                            break;
                        };
                        
                        if (in_region) {
                            draw_hex(grid, tile, BLUE, false);
                        }
                    }
                }
            }
        }
    }
    
    draw_hex_grid(grid);

    /* // draw hilighted_region */
    /* V2 arrow_center = gridV_to_pixel(grid, hilight_region.position); */
    /* draw_grid_arrow(arrow_center, hilight_region.direction, BLUE); */

    // Simulation.
    // Next positon, next position time, swap over.
    // Start by just showing where ya going. Later animate that movement.
    // @INCOMPLETE

    Color button_color = state->level_status == RUNNING ? RED : GREEN;

    if (state->level_status == RUNNING || state->level_status == PAUSED) {
        if (gui_button(state->gui_state, 650, 4, 20, 20, button_color, GUI_ICON_NONE)) {
            state->level_status = state->level_status == RUNNING ? PAUSED : RUNNING;
        }
    }

    if (gui_button(state->gui_state, 775, 4, 20, 20, BLUE, GUI_ICON_NONE)) {
        load_level(state, state->current_level);
    }
    
    draw_formatted_text(v2(675, 20), 24, WHITE, levelstatus_names[state->level_status]);
    draw_formatted_text(v2(800, 20), 24, WHITE, "RESET");

    if (gui_button(state->gui_state, 1200, 4, 20, 20, MAGENTA, GUI_ICON_NONE)) {
        state->current_level--;
        load_level(state, state->current_level);
    }

    if (gui_button(state->gui_state, 1230, 4, 20, 20, MAGENTA, GUI_ICON_NONE)) {
        state->current_level++;
        load_level(state, state->current_level);
    }

    draw_formatted_text(v2(1175, 20), 24, WHITE, "%i", state->current_level);

    if (state->level_status == RUNNING) {
        state->running_time += dt;
    }

    float tick_length = 1.0;
    bool tick_frame = false;

    // If enough time as passed 
    if (state->running_time > state->next_tick) {
        tick_frame = true;
        state->next_tick += tick_length;
    }

    if (tick_frame || state->level_status == PAUSED) {
        state->calculated_thrusters = nodestore_eval(state);
    }

    for (int i=0; i<state->num_entities; i++) {
        Entity* entity = state->entities + i;
        if (entity->id == 0) { continue; }

        switch(entity->type) {
        case(EntityType_SHIP): {
            if (tick_frame) {
                // Move to next position.
                entity->position = entity->next_position;
            }
            // Set thrusters to calculated thrusters.
            entity->thrusters = state->calculated_thrusters;

            // Calculate next position.
            entity->next_position = next_ship_position(entity->position, entity->thrusters);
            
            V2 center = gridV_to_pixel(grid, entity->next_position.tile);

            draw_ship(grid, entity->position, entity->thrusters, RED, YELLOW);
            draw_grid_arrow(center, entity->next_position.facing, WHITE);
        } break;

        case(EntityType_GOAL): {
            draw_goal(grid, entity->position, YELLOW);
        } break;
        }
    }

    // Must have a ship and a goal.
    assert(state->ship_entity && state->goal_entity);
    if (state->ship_entity->position.tile.x == state->goal_entity->position.tile.x &&
        state->ship_entity->position.tile.y == state->goal_entity->position.tile.y &&
        state->ship_entity->position.tile.z == state->goal_entity->position.tile.z) {
        state->level_status = WON;
    }

    // @DEBUG: Draw where the region is supossed to be.
    if (show_hilight_region) {
        V2 arrow_center = gridV_to_pixel(grid, hilight_region.position);
        draw_grid_arrow(arrow_center, hilight_region.direction, CYAN);
    }

    /* draw_hex(grid, mouse_over, CYAN); */

    // show mouse
    /* draw_circle(v2(input.mouse_x, input.mouse_y), 3, GREEN); */
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
