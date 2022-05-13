
#include "flappy_bird.h"
#include "../TouchPanel/TouchPanel.h"
#include "../GLCD/GLCD.h" 
#include "../can/can_lib.h"
#include "../timer/timer.h"

extern uint8_t lobby;
extern uint8_t ch1_count;
extern uint8_t ch2_count;
extern CAN_MSG send_data;
float jump_speed = -6;
float vert_speed = 0;
uint16_t bird_x = 10;
uint16_t bird_y = 150;
uint8_t clicked = 0;


uint8_t timer_scale = 0;

void game_setup(){
	LCD_Clear(Cyan);
	LCD_DrawRect(0, MAX_Y-BOTTOM_SPACE, MAX_X, BOTTOM_SPACE, Green);
}

void game_start(){
	init_timer(0, 0x6108 ); 						  /* 1ms * 25MHz = 25*10^3 = 0x6108 */
	enable_timer(0);
}

void bird_jump(){
	//emit_tone(HIGH_TONE);
}

void bird_fall(){
	// TODO
}

void game_loop(){
	// one every millisecond
	uint8_t res;
	timer_scale++;
	res	= getDisplayPoint(&display, Read_Ads7846(), &matrix );
  if(res){
		uint16_t x = display.x;
		uint16_t y = display.y;
		if(y <= MAX_Y && x <= MAX_X){
			vert_speed = jump_speed;
		} 
	}
	if (timer_scale>=32){
		timer_scale=0;
		clear_bird(bird_x, bird_y, Cyan);
		// manage Y
		bird_y += vert_speed*2;
		if (bird_y + BIRD_HEIGHT + BOTTOM_SPACE > MAX_Y){
			bird_y = 0;
			vert_speed = 0;
			// TODO: manage loosing
		}
		else
			vert_speed += 1;
		// manage X
		bird_x += 2;
		if (bird_x + BIRD_WIDTH > MAX_X){
			bird_x = 0;
			// TODO: manage other screen
		}
		draw_bird(bird_x, bird_y);
	}
}
