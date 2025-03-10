/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:          game.c
** Last modified Date: 
** Last Version:       
** Descriptions:       Main game logic for the Pac-Man-like game, including movement, collision detection, 
**                     scoring, and rendering game elements on the display.
**--------------------------------------------------------------------------------------------------------       
*********************************************************************************************************/

#include "functions.h"

// Game state variables
int score = 0;                 // Current game score
int pills_counter = 0;         // Counter for collected pills
int lives_counter = 1;         // Number of lives remaining
int previous_score = 0;        // Previous score for life bonus tracking
int countdown = 60;            // Game timer countdown
int start_counter = 0;         // Flag to track if the game has started
int endgame = 0;               // Flag to track if the game is over
char s_str[8];                 // Buffer for score number conversion
char c_str[3];                 // Buffer for countdown number conversion
char command;                  // Stores the current movement direction (U, D, L, R)

// Game flags
atomic_int is_blinky_eaten = 0; // Flag to track if Blinky has been eaten
atomic_int is_power_mode = 0;   // Flag to track if power mode is on

// Coordinates for Pac-Man and Blinky
atomic_int pacman_x, pacman_y;  // Pac-Man's position on the map (x, y)
atomic_int blinky_x, blinky_y;  // Blinky's position on the map (x, y)

// Game map definition
char game_map[ROWS][COLS] = {
    "                              ",
    "                              ",
    "                              ",
    "                              ",
    "                              ",
    " #============##============# ",
    " |............||............| ",
    " |.#==#.#===#.||.#===#.#==#.| ",
    " |.|  |.|   |.||.|   |.|  |.| ",
    " |.#==#.#===#.##.#===#.#==#.| ",
    " |..........................| ",
    " |.#==#.##.#======#.##.#==#.| ",
    " |.#==#.||.#==##==#.||.#==#.| ",
    " |......||....||....||......| ",
    " #====#.|#==# || #==#|.#====# ",
    "      |.|#==# ## #==#|.|      ",
    "      |.||          ||.|      ",
    "      |.|| #=#--#=# ||.|      ",
    " #====#.## |      | ##.#====# ",
    " <   ...   |  B   |   ...   > ",
    " #====#.## |      | ##.#====# ",
    "      |.|| #======# ||.|      ",
    "      |.||          ||.|      ",
    "      |.|| #======# ||.|      ",
    " #====#.## #==##==# ##.#====# ",
    " |............||............| ",
    " |.#==#.#===#.||.#===#.#==#.| ",
    " |.#=#|.#===#.##.#===#.|#=#.| ",
    " |...||.......P .......||...| ",
    " #=#.||.##.#======#.##.||.#=# ",
    " #=#.##.||.#==##==#.||.##.#=# ",
    " |......||....||....||......| ",
    " |.#==#.##==#.##.#==##.#==#.| ",
    " |..........................| ",
    " #==========================# ",
    "                              ",
    "                              "
};

// Game over handler function
void gameover() {
    endgame = 1;
    
    // Disable all timers to halt the game
    disable_timer(0);
		disable_timer(1);
		disable_timer(2);
		disable_timer(3);
    
    // Reset timers to initial state
    reset_timer(0);
    reset_timer(1);
    reset_timer(2);
		reset_timer(3);
	
		// Lock the movement if necessary
    move_lock = LOCK_ACQUIRED;

    // Clear screen and display Game Over message
    LCD_Clear(Black); 
    GUI_Text(90, 150, (uint8_t*)"GAMEOVER!", Red, Black);
    
    // Reset countdown timer to its initial value
    countdown = 60;
}

// Victory handler function
void victory() {
    endgame = 1;

    // Disable all timers to halt the game
    disable_timer(0);
		disable_timer(1);
		disable_timer(2);
		disable_timer(3);
    
    // Reset timers to initial state
    reset_timer(0);
    reset_timer(1);
    reset_timer(2);
		reset_timer(3);
    
    // Lock the movement if necessary
    move_lock = LOCK_ACQUIRED;
    
    // Clear screen and display Victory message
    LCD_Clear(Black);
		draw_big_pacman(125, 100, 15, Black, Yellow);
    GUI_Text(95, 150, (uint8_t*)"VICTORY", Yellow, Blue);
    
    // Reset countdown timer to its initial value
    countdown = 60;
}

// Function to draw a filled circle on the LCD (used for Pac-Man, lives, etc.)
void draw_circle(int xpos, int ypos, int r, uint16_t Color) {
    int x, y;
    
    // Use the circle equation (x^2 + y^2 = r^2) to determine which pixels to fill
    for (y = -r; y <= r; y++) {
        for (x = -r; x <= r; x++) {
            if (x * x + y * y <= r * r) {
                TP_DrawPoint(xpos + x, ypos + y, Color);  // Draw a point at the calculated position
            }
        }
    }
}

// Function to remove a life indicator from the display (used when Pac-Man loses a life)
void remove_life() {
    lives_counter -= 1;  // Decrease the remaining lives
    if (lives_counter < 5) {
        // Erase life indicator by drawing a black circle
        draw_circle(58 + lives_counter * 15, 312, 5, Black);
    }
}

// Function to display remaining lives (up to a maximum of 5 lives)
void print_lives() {
    int c;
    
    // Draw yellow circles for each remaining life (max 5)
    for (c = 0; c < lives_counter; c++) {
        draw_circle(58 + c * 15, 312, 5, Yellow);
        if (c == 4) {
            break;  // Stop drawing if 5 lives are displayed
        }
    }
}

// Function to update score based on collected pills or power pills
void compute_score(int x, int y) {
    switch (game_map[y][x]) {
        case PILL:
            score += 10;  // Regular pill (PILLs)
            pills_counter++;  // Increment the pill counter
            break;
        case POWER_PILL:
            score += 50;  // Power pill (PILLs)
            
            // Start power mode if Blinky hasn't been eaten yet
            if (!is_blinky_eaten) {
                disable_timer(2);  // Disable the power mode timer
                reset_timer(2);    // Reset the timer
                init_timer(2, 0, 0, 3, 0.4 * 25000000);  // Set power mode timer
                is_power_mode = PM_ON;  // Set power mode flag
                draw_circle(blinky_x * CELL_SIZE + 4, blinky_y * CELL_SIZE + 4, 4, Blue);  // Draw Blinky in frightened mode
                find_path();  // Update Blinky's path
                enable_timer(2);  // Enable the power mode timer
            }
            pills_counter++;  // Increment the pill counter
            break;
        default:
            break;
    }
    
    // Award extra life every 1000 points
    if (score >= previous_score + 1000) {
        lives_counter += 1;  // Increase life counter
        previous_score += 1000;  // Update the previous score to track for next life
    }
}

// Function to randomly place power pills on the map (6 power pills)
void place_power_pills() {
    int min_x = 6;      // Minimum X coordinate for placement
    int max_x = 36;     // Maximum X coordinate for placement
    int min_y = 2;      // Minimum Y coordinate for placement
    int max_y = 27;     // Maximum Y coordinate for placement
    int pills_placed = 0;  // Counter for placed pills
    int rand_y = 0;     // Random Y position
    int rand_x = 0;     // Random X position
    
    // Use ADC reading as random seed
    int seed = (LPC_ADC->ADGDR >> 4) & 0xFFF;
    srand(seed);  // Initialize random number generator
    
    // Place 6 power pills randomly on valid positions
    while (pills_placed < 6) {
        rand_y = min_y + rand() % (max_y - min_y + 1);  // Generate random Y position
        rand_x = min_x + rand() % (max_x - min_x + 1);  // Generate random X position

        // Check if the position is empty (has a regular pill)
        if (game_map[rand_y][rand_x] == PILL) {
            game_map[rand_y][rand_x] = POWER_PILL;  // Place power pill
            pills_placed++;  // Increment the pills placed counter
        }
    }
}

// Function to draw a big Pac-Man with an open mouth
// The mouth is created by leaving out a segment of the circle
void draw_big_pacman(int xpos, int ypos, int r, uint16_t bodyColor, uint16_t bgColor) {
    int x, y;
    double angle;  // For calculating the angle of each point
    double start_angle = 30.0;  // Start angle of the mouth (in degrees)
    double end_angle = 330.0;   // End angle of the mouth (in degrees)

    // Iterate over the circle's area
    for (y = -r; y <= r; y++) {
        for (x = -r; x <= r; x++) {
            if (x * x + y * y <= r * r) {  // Check if the point is inside the circle
                angle = atan2(y, x) * (180.0 / M_PI);  // Calculate the angle in degrees
                if (angle < 0) angle += 360.0;         // Convert negative angles to positive

                // Draw points only outside the mouth area
                if (angle < start_angle || angle > end_angle) {
                    TP_DrawPoint(xpos + x, ypos + y, bodyColor);
                } else {
                    TP_DrawPoint(xpos + x, ypos + y, bgColor);  // Use background color for the mouth
                }
            }
        }
    }
}

void draw_start_menu() {
    int corner_size = 5;  // Size of each corner square

    // Draw top border
    LCD_DrawLine(10, 10, SCREEN_WIDTH-10, 10, Blue);  // Top horizontal line
    LCD_DrawLine(10, 12, SCREEN_WIDTH-10, 12, Blue);  // Top horizontal line below

    // Top-left corner square
    LCD_DrawLine(10, 10, 10 + corner_size, 10, Blue); // Top-left horizontal
    LCD_DrawLine(10, 10, 10, 10 + corner_size, Blue); // Top-left vertical
    LCD_DrawLine(10 + corner_size, 10, 10 + corner_size, 10 + corner_size, Blue); // Right vertical
    LCD_DrawLine(10, 10 + corner_size, 10 + corner_size, 10 + corner_size, Blue); // Bottom horizontal

    // Top-right corner square
    LCD_DrawLine(SCREEN_WIDTH - 10, 10, SCREEN_WIDTH - 10 - corner_size, 10, Blue); // Top-right horizontal
    LCD_DrawLine(SCREEN_WIDTH - 10, 10, SCREEN_WIDTH - 10, 10 + corner_size, Blue); // Top-right vertical
    LCD_DrawLine(SCREEN_WIDTH - 10 - corner_size, 10, SCREEN_WIDTH - 10 - corner_size, 10 + corner_size, Blue); // Left vertical
    LCD_DrawLine(SCREEN_WIDTH - 10, 10 + corner_size, SCREEN_WIDTH - 10 - corner_size, 10 + corner_size, Blue); // Bottom horizontal

    // Draw bottom border
    LCD_DrawLine(SCREEN_WIDTH-10, SCREEN_HEIGHT-10, 10, SCREEN_HEIGHT-10, Blue);  // Bottom horizontal line
    LCD_DrawLine(SCREEN_WIDTH-10, SCREEN_HEIGHT-8, 10, SCREEN_HEIGHT-8, Blue);  // Bottom horizontal line above

    // Bottom-left corner square
    LCD_DrawLine(10, SCREEN_HEIGHT - 10, 10 + corner_size, SCREEN_HEIGHT - 10, Blue); // Bottom-left horizontal
    LCD_DrawLine(10, SCREEN_HEIGHT - 10, 10, SCREEN_HEIGHT - 10 - corner_size, Blue); // Bottom-left vertical
    LCD_DrawLine(10 + corner_size, SCREEN_HEIGHT - 10, 10 + corner_size, SCREEN_HEIGHT - 10 - corner_size, Blue); // Right vertical
    LCD_DrawLine(10, SCREEN_HEIGHT - 10 - corner_size, 10 + corner_size, SCREEN_HEIGHT - 10 - corner_size, Blue); // Top horizontal

    // Bottom-right corner square
    LCD_DrawLine(SCREEN_WIDTH - 10, SCREEN_HEIGHT - 10, SCREEN_WIDTH - 10 - corner_size, SCREEN_HEIGHT - 10, Blue); // Bottom-right horizontal
    LCD_DrawLine(SCREEN_WIDTH - 10, SCREEN_HEIGHT - 10, SCREEN_WIDTH - 10, SCREEN_HEIGHT - 10 - corner_size, Blue); // Bottom-right vertical
    LCD_DrawLine(SCREEN_WIDTH - 10 - corner_size, SCREEN_HEIGHT - 10, SCREEN_WIDTH - 10 - corner_size, SCREEN_HEIGHT - 10 - corner_size, Blue); // Left vertical
    LCD_DrawLine(SCREEN_WIDTH - 10, SCREEN_HEIGHT - 10 - corner_size, SCREEN_WIDTH - 10 - corner_size, SCREEN_HEIGHT - 10 - corner_size, Blue); // Top horizontal

    // Draw left border
    LCD_DrawLine(10, SCREEN_HEIGHT-10, 10, 10, Blue);  // Left vertical line
    LCD_DrawLine(8, SCREEN_HEIGHT-10, 8, 10, Blue);    // Left vertical line inside

    // Draw right border
    LCD_DrawLine(SCREEN_WIDTH-10, SCREEN_HEIGHT-10, SCREEN_WIDTH-10, 10, Blue);  // Right vertical line
    LCD_DrawLine(SCREEN_WIDTH-8, SCREEN_HEIGHT-10, SCREEN_WIDTH-8, 10, Blue);   // Right vertical line inside

    // Draw Pac-Man character at the center
    draw_big_pacman(125, 100, 15, Black, Yellow);

    // Display "PACMAN" text
    GUI_Text(100, 130, (uint8_t*)"PACMAN", Yellow, Blue);
}


/* Function to initialize and draw the game screen */
void draw_screen() {
    // Clear the screen to set the initial background to black
    LCD_Clear(Black);
    
    // Initialize power pills on first run
    if (start_counter == 0) {
        place_power_pills();  // Function to place power pills on the map
        start_counter++;      // Mark that the game has started
    }
    
    // Draw remaining time on the screen (currently commented out)
    GUI_Text(0, 0, (uint8_t*)"REMAINING TIME: ", White, Black);
    /*
        sprintf(c_str, "%d", countdown);  // Convert countdown to string
        GUI_Text(0, 16, (uint8_t*)c_str, White, Black);  // Display the countdown
    */
    
    // Draw the score on the screen (currently commented out)
    GUI_Text(180, 0, (uint8_t*)"SCORE: ", White, Black);
    /*
        sprintf(s_str, "%d", score);  // Convert score to string
        GUI_Text(180, 16, (uint8_t*)s_str, White, Black);  // Display the score
    */
    
    // Initialize variables for iterating over the game map and drawing elements
    int i, j, xpos, ypos;
    char type;

    // Iterate over the game map to draw each element
    for (i = 0; i < ROWS; i++) {
        for (j = 0; j < COLS; j++) {
            type = game_map[i][j];    // Get the type of the current map element
            xpos = j * CELL_SIZE;     // X position based on column index
            ypos = i * CELL_SIZE;     // Y position based on row index

            // Draw the appropriate game element based on the map type
            switch (type) {
                case PACMAN:
                    // Draw Pac-Man as a yellow circle
                    draw_circle(xpos + 4, ypos + 4, 4, Yellow);
                    pacman_y = i;  // Set Pac-Man's Y position
                    pacman_x = j;  // Set Pac-Man's X position
                    break;

                case BLINKY:
                    // Draw Blinky (the red ghost) as a red circle
                    draw_circle(xpos + 4, ypos + 4, 4, Red);
                    blinky_y = i;  // Set Blinky's Y position
                    blinky_x = j;  // Set Blinky's X position
                    break;

                case WALL:
                    // Draw a corner wall piece using lines
                    xpos += 3;
                    ypos += 3;
                    LCD_DrawLine(xpos, ypos, xpos + 3, ypos, Blue);  // Top line of wall
                    xpos += 3;
                    LCD_DrawLine(xpos, ypos, xpos, ypos + 3, Blue);  // Right line of wall
                    ypos += 3;
                    LCD_DrawLine(xpos, ypos, xpos - 3, ypos, Blue);  // Bottom line of wall
                    xpos -= 3;
                    LCD_DrawLine(xpos, ypos, xpos, ypos - 3, Blue);  // Left line of wall
                    break;

                case HOR_WALL:
                    // Draw a horizontal wall
                    ypos += 3;
                    LCD_DrawLine(xpos, ypos, xpos + 8, ypos, Blue);  // Top horizontal line
                    ypos += 2;
                    LCD_DrawLine(xpos, ypos, xpos + 8, ypos, Blue);  // Bottom horizontal line
                    break;

                case VER_WALL:
                    // Draw a vertical wall
                    xpos += 3;
                    LCD_DrawLine(xpos, ypos, xpos, ypos + 8, Blue);  // Left vertical line
                    xpos += 2;
                    LCD_DrawLine(xpos, ypos, xpos, ypos + 8, Blue);  // Right vertical line
                    break;

                case PILL:
                    // Draw a regular scoring pill
                    xpos += 4;
                    ypos += 4;
                    LCD_DrawLine(xpos, ypos, xpos, ypos + 1, White);  // Pill vertical line
                    xpos += 1;
                    LCD_DrawLine(xpos, ypos, xpos, ypos + 1, White);  // Pill vertical line
                    break;

                case POWER_PILL:
                    // Draw a power pill
                    draw_circle(xpos + 4, ypos + 4, 3, White);  // Power pill as a small white circle
                    break;

                case DOOR:
                    // Draw the ghost house door as a magenta line
                    ypos += 5;
                    LCD_DrawLine(xpos, ypos, xpos + 8, ypos, Magenta);  // Ghost house door line
                    break;

                case EMPTY:
                case TELEPORT_LEFT:
                case TELEPORT_RIGHT:
                    // No drawing needed for empty spaces or teleporters
                    break;
            }
        }
    }
    
    // Draw lives display
    GUI_Text(0, 305, (uint8_t*)"LIVES: ", White, Black);
    //print_lives();  // Function to display the number of remaining lives (currently commented out)
    
    // Find path for Blinky (ghost AI pathfinding)
    find_path();  // Function to update Blinky's path (AI for the ghost)
		
}
