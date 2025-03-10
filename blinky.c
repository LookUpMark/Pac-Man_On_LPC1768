#include "../functions.h"

// Lists for nodes to explore (open_list) and already explored (closed_list)
Node open_list[MAX_NODE];    // Nodes to explore
Node closed_list[MAX_NODE];  // Nodes already explored
int open_list_counter = 0;         // Number of nodes in open_list
int closed_list_counter = 0;       // Number of nodes in closed_list

// Direction vectors for exploring neighbors (right, down, left, up)
int dx[] = {0, 1, 0, -1};  // X-axis direction changes
int dy[] = {1, 0, -1, 0};  // Y-axis direction changes

// Path storage
PathStep path[MAX_NODE];  // Steps in the calculated path
int path_counter = 0;        // Number of steps in the path
int current_step = 0;      // Current step in the path

// Blinky destination
int dest_x = 0, dest_y = 0;

int overflow = 0;
int race_condition = 0;

atomic_int find_lock = 0;  // Locks to prevent concurrent access
atomic_int move_lock = 0;
atomic_int eat_lock = 0;

char previous_cell = EMPTY;  // Stores the previous cell Blinky occupied

// Function to calculate Manhattan distance between two points
int distance(int x1, int y1, int x2, int y2) {
    return abs(x1 - x2) + abs(y1 - y2);  // Return the sum of the absolute differences in x and y coordinates
}

// Adds a node to the open_list
void add_open_list(Node node) {
    if (open_list_counter >= MAX_NODE) {
				overflow++;
        return;  // Prevent adding beyond the maximum limit
    }
    open_list[open_list_counter] = node;  // Store the node in the open_list
    open_list_counter++;  // Increment the open list counter
}

// Adds a node to the closed_list
void add_closed_list(Node node) {
    if (closed_list_counter >= MAX_NODE) {
				overflow++;
        return;  // Prevent adding beyond the maximum limit
    }
    closed_list[closed_list_counter] = node;  // Store the node in the closed_list
    closed_list_counter++;  // Increment the closed list counter
}

// Extracts the node with the lowest F value from the open_list
Node pop_lowest_f() {
    int min_index = 0;  // Index of the node with the lowest F value
    int i, j;

    // Find the node with the lowest F value in the open_list
    for (i = 1; i < open_list_counter; i++) {
        if (open_list[i].f < open_list[min_index].f) {
            min_index = i;
        }
    }

    Node node = open_list[min_index];  // Extract the node with the lowest F value

    // Remove the node from open_list by shifting the remaining elements
    for (j = min_index; j < open_list_counter - 1; j++) {
        open_list[j] = open_list[j + 1];
    }
    open_list_counter--;  // Decrease the open list counter

    return node;
}

// Function to create and initialize a new node
Node createNode(int x, int y, int g, int previous_index) {
    Node node;
    node.x = x;  // Set x-coordinate
    node.y = y;  // Set y-coordinate
    node.g = g;  // Set the g value (cost from the start node)
    node.h = distance(x, y, dest_x, dest_y);  // Set the h value (heuristic distance to the goal)
    node.f = g + node.h;  // Set the f value (sum of g and h)
    node.previous_index = previous_index;  // Set the index of the previous node

    return node;  // Return the created node
}

// Function to check if a position is valid and not in closed_list
int isValidPosition(int x, int y) {
    int j;

    if (x < 0 || x >= COLS || y < 0 || y >= ROWS ||
        game_map[y][x] == '#' || game_map[y][x] == '=' || game_map[y][x] == '|') {
        return 0;  // Invalid position
    }

    // Check if the position is already in the closed_list
    for (j = 0; j < closed_list_counter; j++) {
        if (j >= MAX_NODE) {  // Safety check for overflow
            break;
        }
        if (closed_list[j].x == x && closed_list[j].y == y) {
            return 0;  // Position is in closed_list, not valid
        }
    }

    return 1;  // Valid position
}


// Function to calculate the destination of the path based on power mode
void compute_destination() {
    switch (is_power_mode) {
    case PM_OFF:
        dest_x = pacman_x;  // If power mode is off, set Blinky's destination to Pacman's current position
        dest_y = pacman_y;
        break;
    case PM_ON:
        // If power mode is on, set a predefined destination based on Pacman's position
        if (pacman_x <= COLS / 2 && pacman_y <= ROWS / 2) {
            dest_x = 27;
            dest_y = 33;
        } else if (pacman_x <= COLS / 2 && pacman_y >= ROWS / 2) {
            dest_x = 27;
            dest_y = 6;
        } else if (pacman_x >= COLS / 2 && pacman_y <= ROWS / 2) {
            dest_x = 2;
            dest_y = 33;
        } else if (pacman_x >= COLS / 2 && pacman_y >= ROWS / 2) {
            dest_x = 2;
            dest_y = 6;
        }
        break;
    default:
        dest_x = pacman_x;  // Default fallback destination if no power mode is set
        dest_y = pacman_y;
        break;
    }
}

// Function to find the shortest path to the destination using A* algorithm
void find_path() {
    if (find_lock) {
				race_condition = 1;
        return;  // Prevent concurrent access to this function
    }
    find_lock = LOCK_ACQUIRED;  // Acquire lock

    int i;
    compute_destination();  // Compute the destination for Blinky

    // Reset global variables
    path_counter = 0;
    open_list_counter = 0;
    closed_list_counter = 0;
    current_step = 0;

    // Add starting node (Blinky’s position) to open_list
    add_open_list(createNode(blinky_x, blinky_y, 0, -1));

    // While there are nodes to explore in the open_list
    while (open_list_counter > 0) {
        // Extract the node with the lowest F value
        Node current = pop_lowest_f();

        // If the target node is reached, reconstruct the path
        if (current.x == dest_x && current.y == dest_y) {
            add_closed_list(current);  // Add the target node to the closed_list

            // Reconstruct the path from the destination to the start node
            int current_index = closed_list_counter - 1;  // Index of the last added node

            while (current_index != -1) {
                PathStep step;  // Create a temporary path step
                step.x = closed_list[current_index].x;
                step.y = closed_list[current_index].y;

                path[path_counter++] = step;  // Insert the step into the path

                // Move to the previous node in the closed_list
                current_index = closed_list[current_index].previous_index;
            }

            current_step = path_counter - 1;  // Set the current step to the last step in the path
            find_lock = LOCK_RELEASED;  // Release the lock
            return;  // Path found, exit the function
        }

        add_closed_list(current);  // Add the current node to the closed_list

        // Explore neighbors (right, down, left, up)
        for (i = 0; i < 4; i++) {
            int new_x = current.x + dx[i];
            int new_y = current.y + dy[i];

            // Check if the neighbor position is valid
            if (isValidPosition(new_x, new_y)) {
                // Create a new node for the neighbor and add it to the open_list
                add_open_list(createNode(new_x, new_y, current.g + 1, closed_list_counter - 1));
            }
        }
    }
}

// Moves Blinky along the calculated path
void move_blinky() {
		if (move_lock) {
				race_condition = 1;
				return;
		}
		move_lock = LOCK_ACQUIRED;
		
		if (current_step < 0) {
        move_lock = LOCK_RELEASED;
        return;  // Non ci sono più passi nel percorso
    }
		
    int x_current = blinky_x * CELL_SIZE;
    int y_current = blinky_y * CELL_SIZE;
		
		game_map[blinky_y][blinky_x] = previous_cell;

    // Coordinates of the next step
    int x_next = path[current_step].x * CELL_SIZE;
    int y_next = path[current_step].y * CELL_SIZE;

    // Erase Blinky's current position
    switch (previous_cell) {
        case PILL:
            x_current += 4;
            y_current += 4;
            draw_circle(x_current, y_current, 4, Black);
            LCD_DrawLine(x_current, y_current, x_current, y_current+1, White);
            x_current += 1;
            LCD_DrawLine(x_current, y_current, x_current, y_current+1, White);
            break;

        case POWER_PILL:
            draw_circle(x_current+4, y_current+4, 4, Black);
            draw_circle(x_current+4, y_current+4, 3, White);
            break;

        case DOOR:
            draw_circle(x_current+4, y_current+4, 4, Black);
            y_current += 5;
            LCD_DrawLine(x_current, y_current, x_current+8, y_current, Magenta);
            break;

        default:
            draw_circle(x_current+4, y_current+4, 4, Black);
            break;
    }

    // Move Blinky to the next step
    blinky_x = path[current_step].x;
    blinky_y = path[current_step].y;
		
		previous_cell = game_map[blinky_y][blinky_x];
		game_map[blinky_y][blinky_x] = BLINKY;

    // Draw Blinky in the new position
		switch (is_power_mode) {
			case PM_ON:
					draw_circle(x_next+4, y_next+4, 4, Blue);
					break;
			case PM_OFF:
					draw_circle(x_next+4, y_next+4, 4, Red);
					break;
			default:
					draw_circle(x_next+4, y_next+4, 4, Red);
					break;
		}
		
		// Increment the current step
    current_step--;
		
		move_lock = LOCK_RELEASED;
		
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
}
