/*	example code for cc65, for NES
 *  a break the blocks clone, simplified
 *	using neslib
 *	Doug Fraker 2018
 */	
 


#include "LIB/neslib.h"
#include "LIB/nesdoug.h"
#include "Sprites.h" // holds our metasprite data
#include "breaky.h"


	
	
void main (void) {
	
	ppu_off(); // screen off
	
	// load the palettes
	pal_bg(palette_bg);
	pal_spr(palette_sp);
	
	// use the second set of tiles for sprites
	// both bg and sprites are set to 0 by default
	bank_spr(1);
	
	set_scroll_y(0xff); // shift the bg down 1 pixel
	
	draw_bg();
	
	set_vram_buffer(); // do at least once, sets a pointer to a buffer
	clear_vram_buffer();
	
	// turn on screen
	// ppu_on_all(); // already done in draw_bg()

	
	while (1){
		// infinite loop
		ppu_wait_nmi(); // wait till beginning of the frame
		
		pad1 = pad_poll(0); // read the first controller
		pad1_new = get_pad_new(0); // newly pressed button. do pad_poll first
		
		clear_vram_buffer(); // do at the beginning of each frame
		score_lives_draw();
		
		if(lives01){
			movement();
			draw_sprites();
		}
		else{
			oam_clear(); // game over, just turn everything off
		}
	}
}



void draw_bg(void){
	ppu_off(); // screen off

	vram_adr(NAMETABLE_A);
	// this sets a start position on the BG, top left of screen
	
	vram_unrle(breaky_bg2);
	// this unpacks a compressed full nametable
	
	memcpy (c_map, c1, 256);
	// copy the collision map to c_map
	
	vram_adr(NTADR_A(0,6));
	// sets a start address, skipping the top of the screen
	
	for(temp_y = 0; temp_y < 16; ++temp_y){
		for(temp_x = 0; temp_x < 16; ++temp_x){
			temp1 = (temp_y << 4) + temp_x;
			if((temp_x == 0) || (temp_x == 15)) {
				vram_put(0x10); // wall at the edges
				vram_put(0x10);
			}
			else {
				if(c_map[temp1]){ // if block = yes
					vram_put(0x11); // draw block
					vram_put(0x12);
				}
				else{
					vram_put(0); // else draw blank
					vram_put(0);
				}
			}
		}
	}
	
	ppu_on_all();
}



void draw_sprites(void){
	// clear all sprites from sprite buffer
	oam_clear();

	// reset index into the sprite buffer
	sprid = 0;
	
	// draw 2 metasprites
	sprid = oam_meta_spr(Paddle.X, Paddle.Y, sprid, PaddleSpr);
	sprid = oam_meta_spr(Ball.X, Ball.Y, sprid, BallSpr);
}
	
	
	
void movement(void){
	// paddle move
	if(pad1 & PAD_LEFT){
		Paddle.X -= 2;
		if(Paddle.X < PADDLE_MIN) Paddle.X = PADDLE_MIN;
		
	}
	if(pad1 & PAD_RIGHT){
		Paddle.X += 2;
		if(Paddle.X > PADDLE_MAX) Paddle.X = PADDLE_MAX;

	}
	

	
	
	if(ball_state == BALL_OFF){ // ball is inactive, wait a second
		++ball_count;
		if(ball_count > 60){
			ball_state = BALL_STUCK;
			ball_x_rel = 0x0d;
			ball_count = 0;
			return;
		}
		Ball.Y = 0xff; // off screen
	}
	
	if(ball_state == BALL_STUCK){ // ball is stuck to the paddle
		Ball.X = Paddle.X + ball_x_rel;
		Ball.Y = Paddle.Y - 4;
	
		if(pad1_new & (PAD_A | PAD_B)){ // any new a or b press to start
			ball_state = BALL_ACTIVE;
			ball_direction = GOING_UP;
			if(Ball.X < BALL_MIN) Ball.X = BALL_MIN;
			if(Ball.X > BALL_MAX) Ball.X = BALL_MAX;
			return;
		}
		
	}
	
	if(ball_state == BALL_ACTIVE){
		if(ball_direction == GOING_UP){
			Ball.Y -= 3;
			if(Ball.Y < MAX_UP){
				ball_direction = GOING_DOWN;
			}
		}
		else { // going down
			Ball.Y += 3;
			if(Ball.Y > MAX_DOWN){
				--lives01;
				ball_state = BALL_OFF;
			}
			
			// collision w paddle = stuck
			collision = check_collision(&Ball, &Paddle);
			if(collision){
				ball_state = BALL_STUCK;
				ball_x_rel = Ball.X - Paddle.X;
			}
		}
		
		
		
		
		// collision w blocks
		
		temp_x = (Ball.X + 1) & 0xf0; // tiles are 16 px wide
		temp_y = (Ball.Y + 2) & 0xf8; // tiles only 8 px high
		if(temp_y < 0xaf){ // Y of 0x30 + 16*8 = b0. Ball.Y>b0 = off the c_map
			
			temp1 = (temp_x>>4) + (((temp_y-0x30) << 1) & 0xf0);
			// << 1 because tiles only 8 px high
			
			if(c_map[temp1]){ // hit a block
				hit_block();
				return;
			}
		}
		
		// check a little more to the right
		temp_x = (Ball.X + 4) & 0xf0; // tiles are 16 px wide
		temp_y = (Ball.Y + 2) & 0xf8; // tiles only 8 px high
		if(temp_y < 0xaf){ // Y of 0x30 + 16*8 = b0. Ball.Y>b0 = off the c_map
			
			temp1 = (temp_x>>4) + (((temp_y-0x30) << 1) & 0xf0);
			// << 1 because tiles only 8 px high
			
			if(c_map[temp1]){ // hit a block
				hit_block();
			}
		}
		
	}
}	



void hit_block(void) {
	score01 += 1;
	adjust_score();
	ball_direction = GOING_DOWN;
	Ball.Y +=3;
	c_map[temp1] = 0;
	
	address = get_ppu_addr(0, temp_x, temp_y);
	address = address & 0xfffe; // start with the left tile
	one_vram_buffer(0, address); // tile 0 = blank
	++address;
	one_vram_buffer(0, address); // also the one to the right of it
}



void score_lives_draw(void){
	temp1 = score10 + '0';
	one_vram_buffer(temp1, NTADR_A(10,3));
	
	temp1 = score01 + '0';
	one_vram_buffer(temp1, NTADR_A(11,3));
	
	temp1 = '0';
	one_vram_buffer(temp1, NTADR_A(26,3));
	
	temp1 = lives01 + '0';
	one_vram_buffer(temp1, NTADR_A(27,3));
}



void adjust_score(void){
	if (score01 >= 10){
		score01 -= 10;
		++score10;
	}
}

