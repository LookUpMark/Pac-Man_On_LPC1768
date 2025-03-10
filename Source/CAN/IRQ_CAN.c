/*----------------------------------------------------------------------------
 * Name:    Can.c
 * Purpose: CAN interface for LPC17xx with MCB1700
 * Note(s): see also http://www.port.de/engl/canprod/sv_req_form.html
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2009 Keil - An ARM Company. All rights reserved.
 *----------------------------------------------------------------------------
 */

#include <LPC17xx.h>                  /* LPC17xx definitions */
#include "CAN.h"                      /* LPC17xx CAN adaptation layer */
#include "../GLCD/GLCD.h"
#include "../functions.h"
#include <stdio.h>

// External global variables for CAN communication
extern uint8_t icr; 										// Interrupt status register for both CAN controllers
extern CAN_msg CAN_TxMsg;    /* CAN message for sending */
extern CAN_msg CAN_RxMsg;    /* CAN message for receiving */

// Global variables for game status
volatile uint8_t val_countdown = 0;
volatile uint8_t val_lives = 0;
volatile uint16_t val_score = 0;
char tmp[8];  // Temporary buffer for text output

/*----------------------------------------------------------------------------
  CAN interrupt handler
----------------------------------------------------------------------------*/
void CAN_IRQHandler(void) {

    // Process CAN controller 1 interrupts
    icr = 0;
    icr = (LPC_CAN1->ICR | icr) & 0xFF;               /* Clear interrupts */
	
    // If a message is received on CAN Controller #1
    if (icr & (1 << 0)) {
        // Do nothing here (can be extended with specific actions)
    }

    // If a message is transmitted on CAN Controller #1
    if (icr & (1 << 1)) {
			/* ONLY FOR DEBUG PURPOSE */
			// Retrieve data from the transmitted CAN message
			/*
			if (!endgame) {
        val_countdown = CAN_TxMsg.data[0];
        val_lives = CAN_TxMsg.data[1];
        val_score = (CAN_TxMsg.data[2] << 8);
        val_score = val_score | CAN_TxMsg.data[3];
        
        // Update the countdown display on the screen
        sprintf(tmp, "%d", val_countdown);
        GUI_Text(0, 16, (uint8_t*)"00", Black, Black);  // Clear previous countdown
        GUI_Text(0, 16, (uint8_t*)tmp, White, Black);    // Display new countdown value
        
        // Update the lives display (draw yellow circles for each remaining life, max 5)
        int c;
        for (c = 0; c < val_lives; c++) {
            draw_circle(58 + c * 15, 312, 5, Yellow);
            if (c == 4) {
                break;  // Stop drawing if 5 lives are displayed
            }
        }
        
        // Update the score display
        sprintf(tmp, "%d", val_score);
        GUI_Text(180, 16, (uint8_t*)tmp, Black, Black);  // Clear previous score
        GUI_Text(180, 16, (uint8_t*)tmp, White, Black);  // Display new score
			}
			*/
    }

    // Process CAN controller 2 interrupts
    icr = 0;
    icr = (LPC_CAN2->ICR | icr) & 0xFF;               /* Clear interrupts */

    // If a message is received on CAN Controller #2
    if (icr & (1 << 0)) {
			if (!endgame) {
        CAN_rdMsg(2, &CAN_RxMsg);  // Read the received message
        LPC_CAN2->CMR = (1 << 2);  // Release the receive buffer

        // Retrieve data from the received CAN message
        val_countdown = CAN_RxMsg.data[0];
        val_lives = CAN_RxMsg.data[1];
        val_score = (CAN_RxMsg.data[2] << 8);
        val_score = val_score | CAN_RxMsg.data[3];
        
        // Update the countdown display on the screen
        sprintf(tmp, "%d", val_countdown);
        GUI_Text(0, 16, (uint8_t*)"00", Black, Black);  // Clear previous countdown
        GUI_Text(0, 16, (uint8_t*)tmp, White, Black);    // Display new countdown value
        
        // Update the lives display (draw yellow circles for each remaining life, max 5)
        int c;
        for (c = 0; c < val_lives; c++) {
            draw_circle(58 + c * 15, 312, 5, Yellow);
            if (c == 4) {
                break;  // Stop drawing if 5 lives are displayed
            }
        }
        
        // Update the score display
        sprintf(tmp, "%d", val_score);
        GUI_Text(180, 16, (uint8_t*)tmp, Black, Black);  // Clear previous score
        GUI_Text(180, 16, (uint8_t*)tmp, White, Black);  // Display new score
			}
    }

    // If a message is transmitted on CAN Controller #2 (no action in this example)
    if (icr & (1 << 1)) {
        // Do nothing here (can be extended with specific actions)
    }
}
