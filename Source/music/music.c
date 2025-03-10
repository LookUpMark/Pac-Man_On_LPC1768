#include "music.h"
#include <stdlib.h>
#include "timer.h"
#include "LPC17xx.h"

int k;

void playNote(NOTE note)
{
	if(note.freq != pause)
	{
		k = 25000000/(note.freq*45);
		reset_timer(0);
		init_timer(0, 0, 0, 3, k*8);
		enable_timer(0);
	}
	reset_timer(3);
	init_timer(3, 0, 0, 7, (note.duration*1.5));
	enable_timer(3);
}

int isNotePlaying(void)
{
	return ((LPC_TIM0->TCR != 0) || (LPC_TIM3->TCR != 0));
}
