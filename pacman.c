#include "functions.h"

// Function to handle the loss of a life
void lose_life() {
    // Disable timers to stop game processing while resetting
		disable_timer(2);
    disable_timer(0);
    disable_timer(1);
    reset_timer(0);
    reset_timer(1);
    reset_timer(2);
		
		find_lock = LOCK_ACQUIRED;
		move_lock = LOCK_ACQUIRED;
    
    // Reset Pacman and Blinky's positions on the map
    game_map[pacman_y][pacman_x] = EMPTY;
    game_map[blinky_y][blinky_x] = EMPTY;
		previous_cell = EMPTY;
    
    // Reset Pacman's position to starting coordinates
    pacman_x = 14;
    pacman_y = 28;
    game_map[pacman_y][pacman_x] = PACMAN;
    
    // Decrease lives and check for game over condition
    remove_life();
    if (lives_counter == 0) {
        gameover();  // End the game if no lives are left
        return;
    }
		
		// Reset Blinky's position to starting coordinates
    blinky_x = 14;
    blinky_y = 19;
    game_map[blinky_y][blinky_x] = BLINKY;
    
    // Redraw the game screen and update the pathfinding
		find_path();
    draw_screen();
		
		find_lock = LOCK_RELEASED;
		move_lock = LOCK_RELEASED;
    
    // Enable timers again for game processing
    enable_timer(0);
    enable_timer(1);
    enable_timer(2);
}

// Function to handle when Pacman eats a ghost
void eat_ghost() {
    // Disable the timer for ghost behavior
    disable_timer(2);
    reset_timer(2);
    
    // Update score when ghost is eaten
    score += 100;
    sprintf(s_str, "%d", score);
    GUI_Text(180, 16, (uint8_t*)s_str, Black, Black);
    GUI_Text(180, 16, (uint8_t*)s_str, White, Black);
    
    // Reset Blinky's position to starting coordinates
    blinky_x = 14;
    blinky_y = 19;
    game_map[blinky_y][blinky_x] = BLINKY;
    
    // Update pathfinding after eating ghost
    find_path();
    
    is_blinky_eaten = 1; 				// Mark Blinky as eaten
}

// Function to handle Pacman's movement
void move_pacman() {
    // Calculate current pixel position based on Pacman's cell coordinates
    int x_current = pacman_x * CELL_SIZE;
    int y_current = pacman_y * CELL_SIZE;
    int x_updated, y_updated;

    // Handle different movement commands
    switch (command) {
        case 'U': // Move up
            // Check if the next position is valid (no walls)
            if (game_map[pacman_y-1][pacman_x] != WALL && 
                game_map[pacman_y-1][pacman_x] != HOR_WALL && 
                game_map[pacman_y-1][pacman_x] != VER_WALL) {
                
                disable_timer(0);  // Disable timer to stop game processing during movement
                
                // Compute score based on new position
                compute_score(pacman_x, pacman_y-1);
                
                // Update new pixel coordinates for display
                x_updated = pacman_x * CELL_SIZE;
                y_updated = (pacman_y-1) * CELL_SIZE;
                
                // Update game map with new Pacman position
                game_map[pacman_y][pacman_x] = EMPTY;
                game_map[pacman_y-1][pacman_x] = PACMAN;
                
                // Update screen with Pacman's movement
                draw_circle(x_current+4, y_current+4, 4, Black);  // Clear old position
                draw_circle(x_updated+4, y_updated+4, 4, Yellow);  // Draw new position
                
                // Update Pacman's coordinates
                pacman_y--;
                
                enable_timer(0);  // Re-enable timer for game processing
            }
            break;

        case 'D': // Move down
            // Check if the next position is valid (no walls)
            if (game_map[pacman_y+1][pacman_x] != WALL &&
                game_map[pacman_y+1][pacman_x] != HOR_WALL &&
                game_map[pacman_y+1][pacman_x] != VER_WALL &&
                game_map[pacman_y+1][pacman_x] != DOOR) {
                
                disable_timer(0);  // Disable timer to stop game processing during movement
                
                // Compute score based on new position
                compute_score(pacman_x, pacman_y+1);
                
                // Update new pixel coordinates for display
                x_updated = pacman_x * CELL_SIZE;
                y_updated = (pacman_y+1) * CELL_SIZE;
                
                // Update game map with new Pacman position
                game_map[pacman_y][pacman_x] = EMPTY;
                game_map[pacman_y+1][pacman_x] = PACMAN;
                
                // Update screen with Pacman's movement
                draw_circle(x_current+4, y_current+4, 4, Black);  // Clear old position
                draw_circle(x_updated+4, y_updated+4, 4, Yellow);  // Draw new position
                
                // Update Pacman's coordinates
                pacman_y++;
                
                enable_timer(0);  // Re-enable timer for game processing
            }
            break;

        case 'L': // Move left
            // Check if the next position is a teleport
            if (game_map[pacman_y][pacman_x-1] == TELEPORT_LEFT) {
                disable_timer(0);  // Disable timer to stop game processing during teleport
                
                // Teleport Pacman to the right side of the map
                game_map[pacman_y][pacman_x] = EMPTY;
                pacman_x = COLS - 3;  // Teleport to the right edge
                x_updated = pacman_x * CELL_SIZE;
                y_updated = pacman_y * CELL_SIZE;
                
                // Update game map with new Pacman position
                game_map[pacman_y][pacman_x] = PACMAN;
                
                // Update screen with Pacman's movement
                draw_circle(x_current+4, y_current+4, 4, Black);  // Clear old position
                draw_circle(x_updated+4, y_updated+4, 4, Yellow);  // Draw new position
                
                enable_timer(0);  // Re-enable timer for game processing
            }
            // Check if the next position is valid (no walls)
            else if (game_map[pacman_y][pacman_x-1] != WALL &&
                     game_map[pacman_y][pacman_x-1] != HOR_WALL &&
                     game_map[pacman_y][pacman_x-1] != VER_WALL) {
                
                disable_timer(0);  // Disable timer to stop game processing during movement
                
                // Compute score based on new position
                compute_score(pacman_x-1, pacman_y);
                
                // Update new pixel coordinates for display
                x_updated = (pacman_x-1) * CELL_SIZE;
                y_updated = pacman_y * CELL_SIZE;
                
                // Update game map with new Pacman position
                game_map[pacman_y][pacman_x] = EMPTY;
                game_map[pacman_y][pacman_x-1] = PACMAN;
                
                // Update screen with Pacman's movement
                draw_circle(x_current+4, y_current+4, 4, Black);  // Clear old position
                draw_circle(x_updated+4, y_updated+4, 4, Yellow);  // Draw new position
                
                // Update Pacman's coordinates
                pacman_x--;
                
                enable_timer(0);  // Re-enable timer for game processing
            }
            break;

        case 'R': // Move right
            // Check if the next position is a teleport
            if (game_map[pacman_y][pacman_x+1] == TELEPORT_RIGHT) {
                disable_timer(0);  // Disable timer to stop game processing during teleport
                
                // Teleport Pacman to the left side of the map
								game_map[pacman_y][pacman_x] = EMPTY;
                pacman_x = 2;  // Teleport to the left edge
                x_updated = pacman_x * CELL_SIZE;
                y_updated = pacman_y * CELL_SIZE;
                
                // Update game map with new Pacman position
                game_map[pacman_y][pacman_x] = PACMAN;
                
                // Update screen with Pacman's movement
                draw_circle(x_current+4, y_current+4, 4, Black);  // Clear old position
                draw_circle(x_updated+4, y_updated+4, 4, Yellow);  // Draw new position
                
                enable_timer(0);  // Re-enable timer for game processing
            }
            // Check if the next position is valid (no walls)
            else if (game_map[pacman_y][pacman_x+1] != WALL &&
                     game_map[pacman_y][pacman_x+1] != HOR_WALL &&
                     game_map[pacman_y][pacman_x+1] != VER_WALL) {
                
                disable_timer(0);  // Disable timer to stop game processing during movement
                
                // Compute score based on new position
                compute_score(pacman_x+1, pacman_y);
                
                // Update new pixel coordinates for display
                x_updated = (pacman_x+1) * CELL_SIZE;
                y_updated = pacman_y * CELL_SIZE;
                
                // Update game map with new Pacman position
                game_map[pacman_y][pacman_x] = EMPTY;
                game_map[pacman_y][pacman_x+1] = PACMAN;
                
                // Update screen with Pacman's movement
                draw_circle(x_current+4, y_current+4, 4, Black);  // Clear old position
                draw_circle(x_updated+4, y_updated+4, 4, Yellow);  // Draw new position
                
                // Update Pacman's coordinates
                pacman_x++;
                
                enable_timer(0);  // Re-enable timer for game processing
            }
            break;

        default:
            break;
    }

    // Check for collision with Blinky
    if (blinky_x == pacman_x && blinky_y == pacman_y && !eat_lock) {
				eat_lock = LOCK_ACQUIRED;
        previous_cell = EMPTY;  // Clear previous cell

        // Handle Pacman's interaction with Blinky based on power mode
        switch (is_power_mode) {
            case PM_ON:
                eat_ghost();  // Eat ghost if in power mode
                break;
            case PM_OFF:
                lose_life();  // Lose life if not in power mode
                break;
            default:
                is_power_mode = PM_OFF;  // Reset power mode
                break;
        }
				
				find_path();
				
				eat_lock = LOCK_RELEASED;
    }
    
    // Check for victory condition (all pills collected)
    if (pills_counter == 246) {
        victory();  // Trigger victory if all pills are collected
    }
}
