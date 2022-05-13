
#ifndef _FLAPPYBIRD_H_
#define _FLAPPYBIRD_H_

#define N_SIN 15
#define LOW_TONE 2000
#define HIGH_TONE 400
#define BIRD_HEIGHT 24
#define BIRD_WIDTH 34
#define BOTTOM_SPACE 20


/* Includes ------------------------------------------------------------------*/
#include "LPC17xx.h"

/* Private function prototypes -----------------------------------------------*/				
void game_loop(void);
void game_setup(void);
void draw_bird(uint16_t x, uint16_t y);
void clear_bird(uint16_t x, uint16_t t, uint16_t bg );
void emit_tone(unsigned int intensity);
void game_start(void);
#endif
