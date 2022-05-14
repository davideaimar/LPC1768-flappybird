
#include "flappy_bird.h"
#include <stdio.h>
#include <stdlib.h>
#include "../TouchPanel/TouchPanel.h"
#include "../GLCD/GLCD.h" 
#include "../can/can_lib.h"
#include "../timer/timer.h"

extern uint8_t lobby;
extern uint8_t ch1_count;
extern uint8_t ch2_count;
extern CAN_MSG send_data;
int jump_speed = -6;
int vert_speed;
uint16_t bird_x;
uint16_t bird_y;
uint8_t clicked = 0;
uint8_t timer_scale = 0;
uint8_t pipe_x;
uint8_t pipe_y;

void game_setup(){
	char str[20];
	LCD_Clear(Cyan);
	LCD_DrawRect(0, MAX_Y-BOTTOM_SPACE, MAX_X, BOTTOM_SPACE, BOTTOM_COLOR);			
	sprintf(str, "ID lobby: %d", lobby);
	GUI_Text(MAX_X/2 + 20, MAX_Y-16, (uint8_t *) str, Black, BOTTOM_COLOR);
	sprintf(str, "CH1: %d - CH2: %d", ch1_count, ch1_count);
	GUI_Text(0, MAX_Y-16,(uint8_t *) str, Black, BOTTOM_COLOR);
}

void game_start(uint16_t start_x, uint16_t start_y, uint16_t start_speed){
	bird_x = start_x;
	bird_y = start_y;
	vert_speed = start_speed;
	init_timer(0, 2 * 0x6108 ); 						  /* 1ms * 25MHz = 25*10^3 = 0x6108 */
	enable_timer(0);
	// generate pipe x from 100 to 160
	// generate pipe y from 60 to 170
	pipe_x = rand() % 60 + 100, pipe_y = rand() % 110 + 60;
	draw_pipe(pipe_x, pipe_y);
}

void game_loop(){
	// one every 2 milliseconds
	uint8_t res;
	timer_scale++;
	res	= getDisplayPoint(&display, Read_Ads7846(), &matrix );
  if(res){
		uint16_t x = display.x;
		uint16_t y = display.y;
		if(y <= MAX_Y && x <= MAX_X){
			if (clicked==0){
				vert_speed = jump_speed;
				emit_tone(HIGH_TONE);
				clicked = 50;
			}
		}
	} else {
			clicked = clicked == 0 ? 0 : clicked-1;
		}
	if (timer_scale>=16){
		uint16_t old_x, old_y;
		uint8_t next_step = 0;	// 0 = standard frame refresh; 1 = bird reached last X; 2 = will_lose
		timer_scale=0;
		old_x = bird_x;
		old_y = bird_y;
		
		// new Y
		bird_y += vert_speed*2;
		// new X
		bird_x += 2;
		// increment falling speed
		vert_speed += 1;
		
		if ( (bird_y + BIRD_HEIGHT + BOTTOM_SPACE > MAX_Y) || BIRD_HITTING_PIPE){
			next_step = 2;
		}
		else if (bird_x + BIRD_WIDTH > MAX_X){
			next_step = 1;
		}
		
		if (next_step == 0){
			clear_bird(old_x, old_y, BG_COLOR);
			draw_bird(bird_x, bird_y);
		}
		else if (next_step == 1){
			game_setup();
			game_start(0, bird_y, vert_speed);
		} else if (next_step == 2){
			game_setup();
			game_start(0, 150, 0);
		}
	}
}
