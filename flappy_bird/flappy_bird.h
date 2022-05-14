
#ifndef _FLAPPYBIRD_H_
#define _FLAPPYBIRD_H_

#define N_SIN 15
#define LOW_TONE 2000
#define HIGH_TONE 400
#define BIRD_HEIGHT 24
#define BIRD_WIDTH 34
#define BOTTOM_SPACE 20
#define BG_COLOR 0x7FFF
#define BOTTOM_COLOR 0xF800
#define PIPE_COLOR 0x07E0
#define PIPE_HOLE_HEIGHT 100
#define PIPE_WIDTH 40
#define BIRD_HITTING_PIPE ((BIRD_WIDTH+bird_x >= pipe_x && bird_x < pipe_x + PIPE_WIDTH) \
	&& (bird_y <= pipe_y || bird_y+BIRD_HEIGHT >= pipe_y+PIPE_HOLE_HEIGHT))


/* Includes ------------------------------------------------------------------*/
#include "LPC17xx.h"

/* Private function prototypes -----------------------------------------------*/				
void game_loop(void);
void game_setup(void);
void draw_bird(uint16_t x, uint16_t y);
void clear_bird(uint16_t x, uint16_t t, uint16_t bg );
void emit_tone(unsigned int intensity);
void game_start(uint16_t start_x, uint16_t start_y, uint16_t start_speed);
void draw_pipe(uint16_t x, uint16_t y);
void clear_pipe(uint16_t x, uint16_t y);
#endif
