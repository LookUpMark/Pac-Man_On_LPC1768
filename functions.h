/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           functions.h
** Last modified Date:  
** Last Version:
** Descriptions:        Header file for game functions and global variables
** Correlated files:    
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/

#ifndef __FUNC_H
#define __FUNC_H

/*-------------------------------------------------------------------------------------------------------
 * Includes: Necessary header files for LCD control, touch panel, ADC, and other functionalities.
 *-----------------------------------------------------------------------------------------------------*/
#include "GLCD/GLCD.h"          // Graphics LCD control
#include "TouchPanel.h"      // Touch panel handling
#include "ADC/adc.h"            // Analog-to-digital conversion
#include <stdio.h>              // Standard I/O functions
#include <stdbool.h>            // Boolean support
#include <stdlib.h>             // General utilities
#include <string.h>             // String manipulation
#include "LPC17xx.h"            // LPC17xx-specific definitions
#include "timer.h"              // Timer utilities
#include <stdatomic.h>          // Atomic operations
#include <math.h>               // Math utilities

/*-------------------------------------------------------------------------------------------------------
 * Definitions: Constants for the game, including dimensions and special characters.
 *-----------------------------------------------------------------------------------------------------*/

// Math constants
#define M_PI 3.14159265358979323846 // Pi constant (if not defined)

// Screen dimensions
#define SCREEN_WIDTH 240             // LCD screen width (in pixels)
#define SCREEN_HEIGHT 320            // LCD screen height (in pixels)
#define CELL_SIZE 8                  // Size of each cell in the game grid
#define ROWS (SCREEN_HEIGHT / CELL_SIZE) // Total rows in the grid
#define COLS (SCREEN_WIDTH / CELL_SIZE)  // Total columns in the grid

// Game state constants
#define PM_ON 1                     // Power mode on
#define PM_OFF 0                    // Power mode off
#define LOCK_ACQUIRED 1             // Lock acquired state
#define LOCK_RELEASED 0             // Lock released state

// Game map characters
#define WALL '#'                    // Corner wall piece
#define HOR_WALL '='                // Horizontal wall
#define VER_WALL '|'                // Vertical wall
#define EMPTY ' '                   // Empty space
#define PILL '.'                    // Regular scoring pill
#define POWER_PILL 'o'              // Bonus scoring pill
#define TELEPORT_LEFT '<'           // Left teleport entrance
#define TELEPORT_RIGHT '>'          // Right teleport entrance
#define DOOR '-'                    // Ghost house door
#define PACMAN 'P'                  // Pac-Man character
#define BLINKY 'B'                  // Blinky character

// Other game constants
#define MAX_NODE 500                // Maximum nodes manageable
#define UNREACHABLE 99999999        // Cost for unreachable nodes

/*-------------------------------------------------------------------------------------------------------
 * Data Structures: Types for storing game-related data.
 *-----------------------------------------------------------------------------------------------------*/

// Node for A* pathfinding algorithm
typedef struct {
    int x, y;                  // Node's coordinates
    int g;                     // Cost from start node to current node
    int h;                     // Heuristic cost to the end node
    int f;                     // Total cost (g + h)
    int previous_index;        // Index of the previous node
} Node;

// Steps in a path
typedef struct {
    int x, y;                  // Coordinates of the path step
} PathStep;

/*-------------------------------------------------------------------------------------------------------
 * Global Variables: Externally accessible variables for game state management.
 *-----------------------------------------------------------------------------------------------------*/

// Game map
extern char game_map[ROWS][COLS]; 

// Pac-Man and Blinky positions
extern atomic_int pacman_x; 
extern atomic_int pacman_y;
extern atomic_int blinky_x;
extern atomic_int blinky_y;

// Game status variables
extern char command;                   // Current movement direction (U, D, L, R)
extern int start_counter;              // Game start flag
extern int score;                      // Current score
extern int pills_counter;              // Pills collected
extern int lives_counter;              // Remaining lives
extern int previous_score;             // Previous score for bonus life tracking
extern int countdown;                  // Countdown timer
extern char s_str[8];                  // Score buffer (string)
extern char c_str[3];                  // Countdown buffer (string)

// Pathfinding lists and counters
extern Node open_list[MAX_NODE];       // Open list for A* algorithm
extern Node closed_list[MAX_NODE];     // Closed list for A* algorithm
extern int open_list_counter;          // Count of nodes in open list
extern int closed_list_counter;        // Count of nodes in closed list

// Path and steps
extern PathStep path[MAX_NODE]; 
extern int path_counter;               // Total steps in path
extern int current_step;               // Current step in path

// Atomic game state flags
extern atomic_int is_power_mode;
extern atomic_int is_blinky_eaten;
extern atomic_int find_lock;
extern atomic_int move_lock;
extern atomic_int eat_lock;

// Miscellaneous
extern char previous_cell;             // Previous cell state
extern volatile int music;             // Music flag
extern int endgame;                    // Endgame flag

/*-------------------------------------------------------------------------------------------------------
 * Function Declarations: Prototypes for game functions.
 *-----------------------------------------------------------------------------------------------------*/

// Game management
extern void print_lives();
extern void compute_score(int x, int y);
extern void remove_life();
extern void draw_screen();
extern void place_power_pills();
extern void gameover();
extern void victory();
extern void lose_life();

// Drawing functions
extern void draw_circle(int xpos, int ypos, int r, uint16_t Color);
extern void draw_big_pacman(int xpos, int ypos, int r, uint16_t bodyColor, uint16_t bgColor);
extern void draw_start_menu();

// Movement and logic
extern void move_pacman();
extern void find_path();
extern void eat_ghost();
extern void move_blinky();

#endif /* __FUNC_H */

/****************************************************************************
**                            End Of File
****************************************************************************/
