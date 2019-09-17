

#include "BG/breaky_bg2.h" // generated with NES Screen Tool, with RLE option on
#include "CSV/c1.csv" // collision data, hand typed

#define PADDLE_MIN 0x10
#define PADDLE_MAX 0xd0
#define PADDLE_Y 0xd0
#define MAX_UP 0x30
#define MAX_DOWN 0xe0
#define BALL_MIN 0x10
#define BALL_MAX 0xea


#pragma bss-name(push, "ZEROPAGE")

// GLOBAL VARIABLES
unsigned char pad1;
unsigned char pad1_new;
unsigned char collision;
unsigned char temp1;
unsigned char temp_x;
unsigned char temp_y;

int address;
unsigned char lives01 = 3;
unsigned char score10;
unsigned char score01;
unsigned char ball_state;
enum{
	BALL_OFF, BALL_STUCK, BALL_ACTIVE
};

unsigned char ball_x_rel; // if stuck
unsigned char ball_count; // if off
unsigned char ball_direction;
enum{
	GOING_UP, GOING_DOWN
};




#pragma bss-name(push, "BSS")

unsigned char c_map[256];
// collision map 
// 16 wide has easier math than 14 wide, so sides are padded with 0



struct SpObject {
	unsigned char X;
	unsigned char Y;
	unsigned char width;
	unsigned char height;
};

struct SpObject Paddle = {0x75,PADDLE_Y,0x1f,7};
struct SpObject Ball = {0xff,0xff,5,5}; // balls x and x will be init later










const unsigned char palette_bg[]={
0x0f,0x00,0x10,0x30,
0x0f,0x01,0x21,0x39,
0x0f,0x04,0x24,0x36,
0x0f,0x09,0x29,0x38 
};



const unsigned char palette_sp[]={
0x0f, 0x00, 0x10, 0x30, // greys
0,0,0,0,
0,0,0,0,
0,0,0,0
}; 


// PROTOTYPES
void draw_bg(void);
void draw_sprites(void);
void movement(void);	
void score_lives_draw(void);
void adjust_score(void);
void hit_block(void);
