# Pac-Man for LandTiger LPC1768

This is a complete implementation of the classic Pac-Man game for the **LandTiger LPC1768** development board, developed using **Keil uVision**. The project features modular code, real-time interrupt handling, and an optimized rendering system.

This project was developed for the course **Computer Architectures** in my Master's Degree in **Artificial Intelligence and Data Analytics** at **Politecnico di Torino**.

---

## Features

- **Pac-Man Movement**: Controlled via the joystick, managed by **Timer 0 interrupts**.
- **Ghost AI (Blinky)**: Moves autonomously using the **A* pathfinding algorithm**, with movement controlled by **Timer 2**.
- **Game Map**: Defined as a character array containing walls, empty spaces, normal pills, and power pills.
- **Scoring System**:
  - **10 points** for eating a normal pill.
  - **50 points** for eating a power pill.
- **Power Mode**: Activated upon eating a power pill, allowing Pac-Man to eat ghosts for a limited time, managed by **Timer 2**.
- **Lives System**: Pac-Man starts with a set number of lives, which decrease upon being caught by a ghost.
- **Game Timer**: A countdown timer displayed on the screen, managed by **Timer 1**.
- **Win/Loss Conditions**:
  - The game ends when Pac-Man runs out of lives.
  - The game is won when all pills are collected.

---

## Code Structure

The project is divided into multiple C and header files for clarity and maintainability:

### Libraries for Character Display
- **AsciiLib.c / AsciiLib.h**: Handles 8x16 ASCII characters.
- **HzLib.c / HzLib.h**: Manages GB2312 Chinese characters.

### Hardware Management
- **CAN.h**: Interface for CAN communication, defining the `CAN_msg` structure.
- **GLCD.c / GLCD.h**: Handles LCD display interface.
- **TouchPanel.c / TouchPanel.h**: Manages the touch panel via SPI (SSP1).
- **RIT.h**: Header for RIT interrupts.

### Interrupt Handling
- **IRQ_CAN.c**: Handles CAN communication interrupts.
- **IRQ_RIT.c**: Manages the RIT timer interrupt, used for joystick input.
- **IRQ_timer.c**: Controls game timing, Pac-Man movement, ghost movement, and countdown.

### Game Logic
- **funct_led.c**: Manages LED functions.
- **functions.h**: Declares global variables (countdown, score, path, open_list, closed_list, game state flags).
- **game.c**: Implements scoring system, pill management, and power mode.
- **blinky.c**: Implements Blinky’s movement and pathfinding.
- **pacman.c**: Handles Pac-Man's movement and interactions.

### Supporting Libraries
- **lib_CAN.c**: Implements CAN communication.
- **lib_RIT.c**: Provides RIT timer functions.
- **lib_timer.c**: Manages timer-based events.
- **music.h**: Defines musical notes and durations.
- **joystick.h**: Defines joystick input handling.
- **led.h**: Defines LED control functions.
- **mpu_armv7.h**: Provides Memory Protection Unit (MPU) configuration.

### Main Files
- **sample.c**: The main game logic file.

---

## Detailed Game Logic

### Movement Logic

#### Pac-Man Movement
- **Function**: `move_pacman()` in `pacman.c`
  - Calculates Pac-Man's new position based on the current direction (`UP`, `DOWN`, `LEFT`, `RIGHT`), set by the joystick input handled by the **RIT interrupt** (`IRQ_RIT.c`).
  - Validates the new position by checking the `game_map`. If the next position contains a wall (`WALL`, `HOR_WALL`, `VER_WALL`), the movement is blocked.
  - Updates the `game_map` with Pac-Man's new position and clears the old position by drawing an empty space (`EMPTY`). The position is stored in global variables `pacman_x` and `pacman_y`.
  - **Teleportation**: If Pac-Man moves off the map sides, he reappears on the opposite side.
  - **Pill Interaction**: Calls `compute_score()` if Pac-Man moves onto a cell containing a pill (`PILL` or `POWER_PILL`).
  - **Ghost Interaction**: Checks if Pac-Man is on the same position as Blinky (`blinky_x`, `blinky_y`). Calls `eat_ghost()` if in power mode (`is_power_mode = PM_ON`), otherwise calls `lose_life()`.

#### Blinky Movement
- **Function**: `move_blinky()` in `blinky.c`
  - Responsible for Blinky's movement, based on the path calculated by `find_path()` and stored in the global variable `path`.
  - Movement is controlled by **Timer 2**, moving Blinky towards the next node in the path.
  - Updates the `game_map` with Blinky's new position and clears the old position.
  - **Power Mode**: Changes Blinky's color to blue when power mode is active (`is_power_mode = PM_ON`), otherwise red.
  - **Pac-Man Interaction**: Checks if Blinky is on the same position as Pac-Man. Calls `eat_ghost()` if in power mode, otherwise calls `lose_life()`.

#### Pathfinding (A* Algorithm)
- **Function**: `find_path()` in `blinky.c`
  - Computes the shortest path for Blinky to reach its destination.
  - Destination is determined by `compute_destination()`, which selects Pac-Man's position if power mode is off, otherwise selects a corner of the map.
  - Uses two lists: `open_list` (nodes to explore) and `closed_list` (explored nodes). Functions `add_open_list()` and `add_closed_list()` manage these lists.
  - At each iteration, the node with the lowest cost `f` (sum of `g` and `h`) is extracted from `open_list` using `pop_lowest_f()` and added to `closed_list`.
  - Cost `g` represents the distance from the starting point, while `h` is a heuristic estimate of the distance to the destination, calculated by `distance()`.
  - Once the path is found, it is reconstructed using `previous_index` values and saved in the global variable `path`.
  - Atomic locks `find_lock` and `move_lock` prevent race conditions during execution.

### Game Logic Details

#### Scoring System
- **Function**: `compute_score()` in `game.c`
  - Updates the score when Pac-Man eats a pill.
  - Normal pills (`PILL`) add 10 points, while power pills (`POWER_PILL`) add 50 points and activate power mode.
  - Score is stored in the global variable `score`.
  - Lives are updated graphically using `remove_life()` in `game.c`.
  - Score and lives are updated on the screen using `GLCD`, `AsciiLib`, and `HzLib`.

#### Power Mode
- Activated when Pac-Man eats a power pill (`POWER_PILL`), signaled by `is_power_mode = PM_ON`.
- During power mode, Blinky turns blue in `move_blinky()`.
- Duration is managed by **Timer 2**, setting a specific timer value when power mode is activated. `is_power_mode` is checked in `IRQ_timer.c`.
- When time expires, power mode is deactivated (`is_power_mode = PM_OFF`), and Blinky's pathfinding is recalculated in `IRQ_timer.c`.
- If Pac-Man eats Blinky in power mode, extra points are awarded, and Blinky's path is reset.

#### Lives System
- **Function**: `lose_life()` in `game.c`
  - Called when Pac-Man is caught by Blinky outside power mode.
  - Decrements the life counter (`lives_counter`) and updates the graphical representation by calling `remove_life()` to erase a yellow circle representing a life.
  - If `lives_counter` reaches 0, `gameover()` is called.

#### Game Timer
- Represented by the global variable `countdown`, decremented every second by **Timer 1** in `IRQ_timer.c`.
- When `countdown` reaches 0, `gameover()` is called.
- `previous_countdown` is used to send the current countdown value via CAN in `IRQ_timer.c`.

### Interrupt Handling and CAN Communication

#### Timer 0
- Manages Pac-Man's movement (via `move_pacman()` in the interrupt handler) and decrements `countdown`.
- Also handles music playback by calling `playNote()`. `isNotePlaying()` checks if a note is currently playing.
- Music logic is managed by a software timer in `IRQ_RIT.c` and hardware timers **Timer 0** and **Timer 3**, with functions `init_timer()`, `enable_timer()`, and `reset_timer()` in `lib_timer.c`.
- Note duration is controlled by **Timer 3** using the `duration` parameter in the `NOTE` struct defined in `music.h`.

#### Timer 1
- Manages countdown decrement and displays the game-over message when the timer expires.

#### Timer 2
- Manages Blinky's movement (via `move_blinky()`).
- Controls the duration of power mode.

#### Timer 3
- Sends updated game data (score, lives, and remaining time) via CAN every second in `IRQ_timer.c`. Data is sent using the `CAN_TxMsg` structure and `CAN_wrMsg()` function.
- Synchronizes the duration of musical notes in `music.c` and `IRQ_RIT.c`.

#### RIT (Repetitive Interrupt Timer)
- Manages joystick input reading and sets the command for Pac-Man's movement.

#### CAN (Controller Area Network)
- Implemented with `CAN_wrMsg()` (writes a CAN message) and `CAN_wrFilter()` (configures filters) in `lib_CAN.c`.
- The `CAN_msg` structure in `CAN.h` contains fields `format`, `type`, `id`, `len`, and `data` for message transmission.
- Game data (score, lives, and remaining time) is transmitted cyclically via `CAN_wrMsg()` in `IRQ_timer.c` to communicate with other systems. Message reception is handled by the `CAN_IRQHandler`.
- The global variable `icr` in `IRQ_CAN.c` tracks the status of CAN interrupts.

---

## Compilation and Execution

### Steps to Run the Project
1. Open the project in **Keil uVision**.
2. Set the target to `"LandTiger_LPC1768"`.
3. Compile the project.
4. Connect the **LandTiger board** via USB.
5. Download and execute the program on the board.

---

## Known Issues

- **Ghost AI**: The current implementation of A* search for Blinky can be improved for better performance.
- **Joystick Responsiveness**: Some delay occurs in movement recognition.
- **CAN Communication**: The feature is not fully utilized in the current version.

---

## License

This project is released under the **MIT License**.

---

## Author

Developed by **Marco** as part of an embedded systems project.

If you find this project helpful, feel free to ⭐ the repository! 🚀
