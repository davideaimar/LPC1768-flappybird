
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
extern uint8_t ch1_same_lobby_count;
extern uint8_t ch2_same_lobby_count;
extern CAN_MSG send_data;

const int jump_speed = -6; // constant jump power
int16_t vert_speed;
uint16_t bird_x;
uint16_t bird_y;
uint8_t clicked = 0; // for avoiding keeping toch pressed
uint8_t timer_scale = 0;
uint8_t pipe_x;
uint8_t pipe_y;
uint16_t score = 0;
uint8_t skipped_pipe;

volatile uint8_t GAME_STATUS = 0; 
/* 
	0 = waiting for starting player
	1 = game started but is not in this board
	2 = game going and it's in this board
	3 = game paused and it's in this board
	4 = game over
*/

void game_set(uint16_t start_y, int16_t start_speed, uint16_t initial_score, uint8_t status){
	char str[30];
	
	init_timer(0, 2 * 0x6108 ); // 1ms * 25MHz = 25*10^3 = 0x6108
	disable_timer(0);
	
	bird_x =  0;
	bird_y = start_y;
	vert_speed = start_speed;
	skipped_pipe = 0;
	score = initial_score;
	GAME_STATUS = status;
	
	if (vert_speed == 0)
		vert_speed = jump_speed;
	
	draw_bg();
	draw_bottom_line();
	
	if (GAME_STATUS==4){
		GUI_Text(80, 140, (uint8_t *) "GAME OVER", Yellow, BG_COLOR);
		sprintf(str, "Points: %d", score);
		GUI_Text(80, 160, (uint8_t *) str, Yellow, BG_COLOR);
		update_screen_score();
		clicked = 50;
	}
		
	if (GAME_STATUS == 0 || GAME_STATUS == 2) {
		// generate pipe x from 100 to 160
		// generate pipe y from 60 to 170
		pipe_x = rand() % 60 + 100, pipe_y = rand() % 110 + 60;
		draw_pipe(pipe_x, pipe_y);
		draw_bird(bird_x, bird_y);
	}
	
	if (GAME_STATUS==1){
		GUI_Text(30, 140, (uint8_t *) "Wait for other player", Yellow, BG_COLOR);
		update_screen_score();
	} else {
		enable_timer(0);
	}
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
				emit_tone(MID_TONE);
				clicked = 50;
				if (GAME_STATUS == 4){
					game_set(150, 0, 0, 0);
				}
				else if (GAME_STATUS==0){
					GAME_STATUS = 2;
					FlappyCAN_Send1();
				}
			}
		}
	} else {
		clicked = clicked == 0 ? 0 : clicked-1;
	}
	if (timer_scale>=16 && GAME_STATUS == 2){
		uint16_t old_x, old_y;
		uint8_t next_step = 0;	// 0 = standard frame refresh; 1 = bird reached last X; 2 = will_lose
		timer_scale=0;
		old_x = bird_x;
		old_y = bird_y;
		
		// new Y
		bird_y += vert_speed*2;
		bird_y = bird_y >= 0xFFF0 ? 0 : bird_y; // prevent overflow
		// new X
		bird_x += 2;
		// increment falling speed
		vert_speed += 1;
		if ( skipped_pipe == 0 && bird_x > pipe_x + PIPE_WIDTH){
			char str[10];
			skipped_pipe = 1;
			score++;
			update_screen_score();
			emit_tone(HIGH_TONE);
		}
		if ( (bird_y + BIRD_HEIGHT + BOTTOM_SPACE > MAX_Y) || ( skipped_pipe == 0 &&  BIRD_HITTING_PIPE ) ){
			next_step = 2;
			emit_tone(LOW_TONE);
		}
		else if (bird_x + BIRD_WIDTH > MAX_X){
			next_step = 1;
		}
		
		if (next_step == 0){
			clear_bird(old_x, old_y);
			draw_bird(bird_x, bird_y);
		}
		else if (next_step == 1){
			if (ch1_same_lobby_count > 0 || ch2_same_lobby_count > 0 ) {
				// if there is someone playing with me, send the packet
				FlappyCAN_Send2(bird_y, vert_speed, score);
				game_set(150, 0, score, 1);
			} else {
				// otherwise play alone
				game_set(bird_y, vert_speed, score, 2);
			}
		} else if (next_step == 2){
			FlappyCAN_Send3(score);
			game_set(150, 0, score, 4);
		}
	}
}
