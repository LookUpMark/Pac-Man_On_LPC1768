/*----------------------------------------------------------------------------
 * Name:    sample.c
 * Purpose: 
 *		to control led11 and led 10 through EINT buttons (similarly to project 03_)
 *		to control leds9 to led4 by the timer handler (1 second - circular cycling)
 * Note(s): this version supports the LANDTIGER Emulator
 * Author: 	Paolo BERNARDI - PoliTO - last modified 15/12/2020
 *----------------------------------------------------------------------------
 *
 * This software is supplied "AS IS" without warranties of any kind.
 *
 * Copyright (c) 2017 Politecnico di Torino. All rights reserved.
 *----------------------------------------------------------------------------*/

                  
#include <stdio.h>
#include "LPC17xx.h"                    /* LPC17xx definitions                */
#include "led/led.h"
#include "button_EXINT/button.h"
#include "timer/timer.h"
#include "RIT/RIT.h"
#include "joystick/joystick.h"
#include "ADC/adc.h"
#include "GLCD/GLCD.h" 
#include "TouchPanel.h"
#include "../functions.h"
#include "CAN.h"

#ifdef SIMULATOR
extern uint8_t ScaleFlag; // <- ScaleFlag needs to visible in order for the emulator to find the symbol (can be placed also inside system_LPC17xx.h but since it is RO, it needs more work)
#endif

extern int pause;

/*----------------------------------------------------------------------------
  Main Program
 *----------------------------------------------------------------------------*/
int main (void) {
	SystemInit();  												/* System Initialization (i.e., PLL)  */
  //LED_init();                         /* LED Initialization                 */
  CAN_Init();
	BUTTON_init();												/* BUTTON Initialization              */
	joystick_init();											/* Joystick Initialization            */
	init_RIT(0x004C4B40);									/* RIT Initialization 50 msec         */
	enable_RIT();													/* enable RIT to count 50 msec		 		*/
	ADC_init();													  /* ADC Inizialization 								*/
	
	/******************TIMER ON**********************/
	LPC_SC -> PCONP |= (1 << 22);  // TURN ON TIMER 2
	LPC_SC -> PCONP |= (1 << 23);  // TURN ON TIMER 3	
	
	/******************LCD**********************/
	LCD_Initialization();
	TP_Init();
	LCD_Clear(Black);

	/******************TIMER INIT**********************/
	/* Counter = Period [s] * Frequency [Hz]      */
	/* Counter = 1s * 25Mhz = 25.000.000 = 0xXXXX */
	/* uint32_t init_timer ( uint8_t timer_num, uint32_t Prescaler, uint8_t MatchReg, uint8_t SRImatchReg, uint32_t TimerInterval ) */
	/*
	init_timer(0, 0, 0, 3, 0.015*25000000); 	// Timer 0 MR0 for movement
	init_timer(1, 0, 0, 3, 0.1*1*25000000); 	// Timer 1 MR0 for countdown (1 second scaled for emulation needs)
	init_timer(2, 0, 0, 3, 0.015*25000000); 	// Timer 2 MR0 for Blinky movement
	*/
	
	/* Start from an initial screen (we do not start playing immediately) */
	draw_start_menu();
	pause = 1;
	
	LPC_SC->PCON |= 0x1;										/* power-down	mode										*/
	LPC_SC->PCON &= 0xFFFFFFFFD;			

	LPC_PINCON->PINSEL1 |= (1<<21);				
	LPC_PINCON->PINSEL1 &= ~(1<<20);
	LPC_GPIO0->FIODIR |= (1<<26);						/* Set P0.26 with Output Mode */
		
  while (1) {                           	/* Loop forever                       */	
		__ASM("wfi");
  }

}