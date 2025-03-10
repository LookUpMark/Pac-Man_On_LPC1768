/*********************************************************************************************************
**--------------File Info---------------------------------------------------------------------------------
** File name:           IRQ_timer.c
** Last modified Date:  2014-09-25
** Last Version:        V1.00
** Descriptions:        Functions to manage T0 and T1 interrupts
** Correlated files:    timer.h
**--------------------------------------------------------------------------------------------------------
*********************************************************************************************************/
#include "LPC17xx.h"
#include "timer.h"
#include "../GLCD/GLCD.h"
#include "TouchPanel.h"
#include "../functions.h"
#include <stdio.h>
#include <string.h>
#include "CAN.h"

// External variables defined elsewhere in the program
extern unsigned char led_value;                 /* LED control variable */
extern int countdown;                             /* Countdown timer value */
extern char c_str[3];                            /* String for displaying countdown */

// Local variables
static int c = 0;                                /* General-purpose counter */
static int num_moves = 0;                        /* Counter for Blinky's moves */
static int blinky_respawn_timer = 3;             /* Timer for Blinky's respawn */
static int power_mode_countdown = 10;            /* Countdown for power mode */
static uint16_t SinTable[45] = {                 /* Sine wave lookup table for DAC output */
    410, 467, 523, 576, 627, 673, 714, 749, 778,
    799, 813, 819, 817, 807, 789, 764, 732, 694,
    650, 602, 550, 495, 438, 381, 324, 270, 217,
    169, 125, 87 , 55 , 30 , 12 , 2  , 0  , 6  ,
    20 , 41 , 70 , 105, 146, 193, 243, 297, 353
};

/****************************************************************************** 
** Function name:        Timer0_IRQHandler
**
** Descriptions:         Timer/Counter 0 interrupt handler
**
** Parameters:            None
** Returned value:       None
******************************************************************************/ 
void TIMER0_IRQHandler(void) 
{
    // Check if the interrupt was caused by Match Register 0
    if(LPC_TIM0->IR & 1) {
        if(!music) {
            move_pacman();                         // Move Pacman
        } else {
            static int sineticks = 0;             // Sine wave table tick counter
            static int currentValue;              // Current DAC value
            currentValue = SinTable[sineticks];
            currentValue -= 410;
            currentValue /= 1;
            currentValue += 410;
            LPC_DAC->DACR = currentValue << 6;    // Set DAC output

            // Update sine table tick counter
            sineticks++;
            if(sineticks == 45) {
                sineticks = 0;                   // Reset sine wave counter after completing the cycle
            }
        }
        LPC_TIM0->IR = 1;                          // Clear interrupt flag for MR0
    }
    // Handle other match registers (MR1, MR2, MR3) if necessary
    else if(LPC_TIM0->IR & 2) {
        LPC_TIM0->IR = 2;                          // Clear interrupt flag for MR1
    }
    else if(LPC_TIM0->IR & 4) {
        LPC_TIM0->IR = 4;                          // Clear interrupt flag for MR2
    }
    else if(LPC_TIM0->IR & 8) {
        LPC_TIM0->IR = 8;                          // Clear interrupt flag for MR3
    }

    return;
}

/****************************************************************************** 
** Function name:        Timer1_IRQHandler
**
** Descriptions:         Timer/Counter 1 interrupt handler
**
** Parameters:            None
** Returned value:       None
******************************************************************************/ 
void TIMER1_IRQHandler(void)
{
    // Check if the interrupt was caused by Match Register 0
    if(LPC_TIM1->IR & 1) {
        countdown--;                               // Decrement countdown timer
        if(countdown == 0) {
            disable_timer(0);                      // Disable all timers on game over
            disable_timer(1);
            disable_timer(2);
            gameover();                             // End the game
        }
        
        // Manage power mode and Blinky's respawn timer
        if(is_power_mode) {
            if(is_blinky_eaten) {
                blinky_respawn_timer--;
                if(blinky_respawn_timer == 0) {
                    blinky_respawn_timer = 3;      // Reset respawn timer
                    power_mode_countdown = 10;
                    is_power_mode = PM_OFF;       // Exit power mode
                    is_blinky_eaten = 0;           // Reset Blinky status
                    find_path();                   // Recalculate Blinky's path
                    init_timer(2, 0, 0, 3, 0.25 * 25000000); // Reinitialize Timer 2
                    enable_timer(2);
                }
            } else {
                power_mode_countdown--;
                if(power_mode_countdown == 0) {
                    power_mode_countdown = 10;    // Reset power mode timer
                    disable_timer(2);
                    reset_timer(2);                // Reset Timer 2
                    is_power_mode = PM_OFF;       // Exit power mode
                    find_path();                   // Recalculate Blinky's path
                    init_timer(2, 0, 0, 3, 0.25 * 25000000);
                    enable_timer(2);
                }
            }
        }

        LPC_TIM1->IR = 1;                          // Clear interrupt flag for MR0
    }
    // Handle other match registers (MR1, MR2, MR3) if necessary
    else if(LPC_TIM1->IR & 2) {
        LPC_TIM1->IR = 2;                          // Clear interrupt flag for MR1
    }
    else if(LPC_TIM1->IR & 4) {
        LPC_TIM1->IR = 4;                          // Clear interrupt flag for MR2
    }
    else if(LPC_TIM1->IR & 8) {
        LPC_TIM1->IR = 8;                          // Clear interrupt flag for MR3
    }

    return;
}

/****************************************************************************** 
** Function name:        Timer2_IRQHandler
**
** Descriptions:         Timer/Counter 2 interrupt handler
**
** Parameters:            None
** Returned value:       None
******************************************************************************/ 
void TIMER2_IRQHandler(void)
{
    // Check if the interrupt was caused by Match Register 0
    if(LPC_TIM2->IR & 1) {
        move_blinky();                             // Move Blinky along its path
        num_moves++;                               // Increment move counter

        if(num_moves == 3) {                       // Recalculate path every 3 moves
            num_moves = 0;
            find_path();
        }
        LPC_TIM2->IR = 1;                          // Clear interrupt flag for MR0
    }
    // Handle other match registers (MR1, MR2, MR3) if necessary
    else if(LPC_TIM2->IR & 2) {
        LPC_TIM2->IR = 2;                          // Clear interrupt flag for MR1
    }
    else if(LPC_TIM2->IR & 4) {
        LPC_TIM2->IR = 4;                          // Clear interrupt flag for MR2
    }
    else if(LPC_TIM2->IR & 8) {
        LPC_TIM2->IR = 8;                          // Clear interrupt flag for MR3
    }

    return;
}

/****************************************************************************** 
** Function name:        Timer3_IRQHandler
**
** Descriptions:         Timer/Counter 3 interrupt handler
**
** Parameters:            None
** Returned value:       None
******************************************************************************/ 
void TIMER3_IRQHandler(void)
{
    // Check if the interrupt was caused by Match Register 0
    if(LPC_TIM3->IR & 1) {
        if(music) {
            disable_timer(0);                      // Disable timer if music is playing
        }
        LPC_TIM3->IR = 1;                          // Clear interrupt flag for MR0
    }
    // Check if the interrupt was caused by Match Register 1
    else if(LPC_TIM3->IR & 2) {
        static uint16_t previous_score = -1;
        static uint8_t previous_countdown = -1;
        static uint8_t previous_lives = -1;

        // Check if score, lives, or countdown has changed
        if(score != previous_score || lives_counter != previous_lives || countdown != previous_countdown) {
            previous_lives = lives_counter;
            previous_score = score;
            previous_countdown = countdown;

            // Send updated game data via CAN
            CAN_TxMsg.data[0] = previous_countdown;
            CAN_TxMsg.data[1] = previous_lives;
            CAN_TxMsg.data[2] = (previous_score >> 8) & 0xFF;
            CAN_TxMsg.data[3] = previous_score & 0xFF;
            CAN_TxMsg.len = 4;
            CAN_TxMsg.id = 2;
            CAN_TxMsg.format = STANDARD_FORMAT;
            CAN_TxMsg.type = DATA_FRAME;
            CAN_wrMsg(1, &CAN_TxMsg);
        }
        LPC_TIM3->IR = 2;                          // Clear interrupt flag for MR1
    }
    // Handle other match registers (MR2, MR3) if necessary
    else if(LPC_TIM3->IR & 4) {
        LPC_TIM3->IR = 4;                          // Clear interrupt flag for MR2
    }
    else if(LPC_TIM3->IR & 8) {
        LPC_TIM3->IR = 8;                          // Clear interrupt flag for MR3
    }

    return;
}

/****************************************************************************** 
**                            End Of File
******************************************************************************/ 
