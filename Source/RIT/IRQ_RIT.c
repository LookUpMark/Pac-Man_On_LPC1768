/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_RIT.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        functions to manage T0 and T1 interrupts
** Correlated files:    RIT.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "LPC17xx.h"
#include "RIT.h"
#include "../led/led.h"
#include "timer/timer.h"
#include "GLCD/GLCD.h"
#include "../functions.h"
#include "../music.h"

/******************************************************************************
** Function name:		RIT_IRQHandler
**
** Descriptions:		REPETITIVE INTERRUPT TIMER handler
**
** parameters:			None
** Returned value:		None
**
******************************************************************************/
// Global flags and state variables for the game and music management
volatile int down_0 = 0;
volatile int down_1 = 0;
volatile int down_2 = 0;
volatile int pause = 0;
volatile int music = 1;

// Define the song as a series of notes and their durations
NOTE song[] = {
    {REST, time_minima},
    {NOTE_B2, time_semicroma},
    {NOTE_B3, time_semicroma},
    {NOTE_FS3, time_semicroma},
    {NOTE_DS3, time_semicroma},
    {NOTE_B3, time_semicroma},
    {NOTE_FS3, time_croma * 1.5},
    {NOTE_DS3, time_croma},
    {NOTE_DS3, time_biscroma},
    {NOTE_C3, time_semicroma},
    {NOTE_C4, time_semicroma},
    {NOTE_G3, time_semicroma},
    {NOTE_E3, time_semicroma},
    {NOTE_C4, time_semicroma},
    {NOTE_G3, time_croma * 1.5},
    {NOTE_E3, time_croma},
    {NOTE_B2, time_semicroma},
    {NOTE_B3, time_semicroma},
    {NOTE_FS3, time_semicroma},
    {NOTE_DS3, time_semicroma},
    {NOTE_B3, time_semicroma},
    {NOTE_FS3, time_croma * 1.5},
    {NOTE_DS3, time_croma},
    {NOTE_DS3, time_biscroma},
    {NOTE_E3, time_biscroma},
    {NOTE_F3, time_biscroma},
    {NOTE_F3, time_biscroma},
    {NOTE_FS3, time_biscroma},
    {NOTE_G3, time_biscroma},
    {NOTE_G3, time_biscroma},
    {NOTE_GS3, time_biscroma},
    {NOTE_A3, time_semicroma},
    {NOTE_B3, time_croma}
};

// Main interrupt handler for the Repetitive Interrupt Timer (RIT)
void RIT_IRQHandler(void)
{
    // Static variables for state management
    static uint8_t position = 0; // Position of the LED or game state
    static int select = 0;       // Joystick select button state
    static int down = 0;         // Joystick down button state
    static int left = 0;         // Joystick left button state
    static int right = 0;        // Joystick right button state
    static int up = 0;           // Joystick up button state

    // Music control: Play notes if the song is not completed
    if (music)
    {
        static int currentNote = 0; // Tracks the current note index
        static int ticks = 0;       // Tracks timing between notes

        if (!isNotePlaying()) // Check if no note is currently playing
        {
            ++ticks; // Increment tick counter
            if (ticks == UPTICKS) // Check if it's time to play the next note
            {
                ticks = 0; // Reset ticks
                playNote(song[currentNote++]); // Play the current note and move to the next
            }
        }

        // If the song is finished, disable the music
        if (currentNote == (sizeof(song) / sizeof(song[0])))
        {
            music = 0;
            disable_timer(0);
            disable_timer(3);
            reset_timer(0);
            reset_timer(3);

            // Reinitialize timers for game or interaction
            init_timer(0, 0, 0, 3, 0.25 * 25000000);
            init_timer(1, 0, 0, 3, 1 * 25000000);
            init_timer(2, 0, 0, 3, 0.25 * 25000000);
            init_timer(3, 0, 0, 0, 60 * 25000000);
            init_timer(3, 0, 1, 3, 0.3 * 25000000);

            // Display message prompting to press INT0 to start the game
            GUI_Text(80, 180, (uint8_t *)"Press INT0", White, Black);
						GUI_Text(55, 200, (uint8_t *)"to start the game", White, Black);

        }
    }

    /*************************JOYSTICK_SELECT***************************/
    if ((LPC_GPIO1->FIOPIN & (1 << 25)) == 0)
    {
        /* Joystick SELECT pressed */
        select++;
        // Do nothing for now on SELECT press
    }
    else
    {
        select = 0;
    }

    /*************************JOYSTICK_DOWN***************************/
    if ((LPC_GPIO1->FIOPIN & (1 << 26)) == 0)
    {
        /* Joystick DOWN pressed */
        down++;
        command = 'D'; // Update the command for DOWN action
    }
    else
    {
        down = 0;
    }

    /*************************JOYSTICK_LEFT***************************/
    if ((LPC_GPIO1->FIOPIN & (1 << 27)) == 0)
    {
        /* Joystick LEFT pressed */
        left++;
        command = 'L'; // Update the command for LEFT action
    }
    else
    {
        left = 0;
    }

    /*************************JOYSTICK_RIGHT***************************/
    if ((LPC_GPIO1->FIOPIN & (1 << 28)) == 0)
    {
        /* Joystick RIGHT pressed */
        right++;
        command = 'R'; // Update the command for RIGHT action
    }
    else
    {
        right = 0;
    }

    /*************************JOYSTICK_UP***************************/
    if ((LPC_GPIO1->FIOPIN & (1 << 29)) == 0)
    {
        /* Joystick UP pressed */
        up++;
        command = 'U'; // Update the command for UP action
    }
    else
    {
        up = 0;
    }

    /*************************INT0***************************/
    if (down_0 != 0)
    {
        down_0++;
        if ((LPC_GPIO2->FIOPIN & (1 << 10)) == 0)
        {
            /*
            switch (pause) {
                    case 0:
                            disable_timer(0);
                            disable_timer(1);
                            disable_timer(2);
                            disable_timer(3);
                            LCD_Clear(Black);
                            GUI_Text(100, 140, (uint8_t *)"PAUSE", Black, White);
                            pause = 1;
                            break;
                    case 1:
                            draw_screen();
                            enable_timer(0);
                            enable_timer(1);
                            enable_timer(2);
                            enable_timer(3);
                            pause = 0;
                            break;
            }
            */
            // If the game is not in music mode and not paused, toggle pause state
            if (!pause && !music && !endgame)
            {
                disable_timer(0);
                disable_timer(1);
                disable_timer(2);
                disable_timer(3);
                LCD_Clear(Black);
								draw_start_menu();
                GUI_Text(105, 160, (uint8_t *)"PAUSE", Black, White);
                pause = 1;
            }
            else if (pause && !music && !endgame)
            {
                draw_screen();
                enable_timer(0);
                enable_timer(1);
                enable_timer(2);
                enable_timer(3);
                pause = 0;
            }
        }
        else
        { /* Button released */
            down_0 = 0;
            NVIC_EnableIRQ(EINT0_IRQn); // Enable external interrupt for INT0
            LPC_PINCON->PINSEL4 |= (1 << 20); /* External interrupt 0 pin selection */
        }
    } // End INT0

    // The following sections for KEY1 and KEY2 are commented out.
    // If required, they can be uncommented for handling other button presses.

    // /*************************KEY1***************************/
    // if (down_1 != 0) {
    //   down_1++;
    //   if ((LPC_GPIO2->FIOPIN & (1 << 11)) == 0) {
    //     switch (down_1) {
    //       case 2:
    //         if (position == 7) {
    //           LED_On(0);
    //           LED_Off(7);
    //           position = 0;
    //         } else {
    //           LED_Off(position);
    //           LED_On(--position);
    //         }
    //         break;
    //       default:
    //         break;
    //     }
    //   } else {
    //     down_1 = 0;
    //     NVIC_EnableIRQ(EINT1_IRQn); // Enable external interrupt for KEY1
    //     LPC_PINCON->PINSEL4 |= (1 << 22); /* External interrupt 1 pin selection */
    //   }
    // }

    // /*************************KEY2***************************/
    // if (down_2 != 0) {
    //   down_2++;
    //   if ((LPC_GPIO2->FIOPIN & (1 << 12)) == 0) {
    //     switch (down_2) {
    //       case 2:
    //         if (position == 7) {
    //           LED_On(0);
    //           LED_Off(7);
    //           position = 0;
    //         } else {
    //           LED_Off(position);
    //           LED_On(++position);
    //         }
    //         break;
    //       default:
    //         break;
    //     }
    //   } else {
    //     down_2 = 0;
    //     NVIC_EnableIRQ(EINT2_IRQn); // Enable external interrupt for KEY2
    //     LPC_PINCON->PINSEL4 |= (1 << 24); /* External interrupt 2 pin selection */
    //   }
    // }

    /*************************ADC***************************/
    ADC_start_conversion(); // Start ADC conversion

    // Clear the interrupt flag for the RIT
    LPC_RIT->RICTRL |= 0x1;

    // Reset the RIT and clear the interrupt flag again
    reset_RIT();
    LPC_RIT->RICTRL |= 0x1;

    return;
}
/******************************************************************************
**                            End Of File
******************************************************************************/ 
