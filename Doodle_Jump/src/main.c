/*******************************************************************************
List of header files
*******************************************************************************/
#include <psp2/kernel/processmgr.h>
#include <psp2/touch.h>
#include <psp2/ctrl.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h> 


#include <vita2d.h>

/*******************************************************************************
Defines
*******************************************************************************/
#define WHITE RGBA8(255, 255, 255, 255)
#define BLACK RGBA8(0, 0, 0, 255)
#define GREEN RGBA8(0,186,6,255)
#define RED RGBA8(255,0,0,255)

#define SKY_BLUE RGBA8(43, 144, 191, 255)
#define CLEARED_GREEN RGBA8(58, 178, 74, 255)
/**********************     Object size defines   *****************************/
/*for some reason the touch screen sizes are double*/
#define PSVITA_TOUCH_WIDTH 1920
#define PSVITA_TOUCH_HEIGHT 1088

#define PSVITA_SCREEN_WIDTH 960
#define PSVITA_SCREEN_HEIGHT 544

#define PLATFORM_WIDTH 100
#define PLATFORM_HEIGHT 30

#define PLAYER_WIDTH 50
#define PLAYER_HEIGHT 80

#define INITAL_PLAT_COUNT 8

/**********************   Physics Constants defines****************************/
#define TIME 1/40
#define GRAVITY 4

/*************************   Collision defines     ****************************/
#define PLATFORM_LANDING_SENSITIVITY 5 //The pixel gap at which collision with a platform is detected


/*******************************************************************************
Structs
*******************************************************************************/
enum platformType 
{
	NORMAL,
};

struct Position
{
    float x;
    float y;
};


struct Platform
{
    struct Position pos;
    int width;
    int height;
    enum platformType type;
    unsigned int colour;
};

struct Player
{
	struct Position pos;
	struct Position vel;
	int width;
	int height;
	int mass;
	unsigned int colour;
};

typedef struct Platform Platform_t;
typedef struct Player Player_t;

/* TODO: why touch[port].report[i].force is always at 128 ? */

/*******************************************************************************
Function prototypes
*******************************************************************************/

void setUpTouchScreen();

void generatePlatforms(Platform_t platforms[], int numPlat);

void handleTouch(Player_t *player, SceTouchData touch[]);
void handlePhysics(Player_t *player);


void handleCollisionChecking(Player_t *player, Platform_t platArray[], int platformLength);
int isOnPlatform(float playerX, float playerY, float playerWidth, float playerHeight, 
				 float platformX, float platformY, float platformWidth, float platformHeight);

int testInRange(int numberToCheck, int bottom, int top);

void drawPlayer(Player_t player);
void drawPlatforms(Platform_t platforms[], int platformLength);
void drawBorder(Platform_t platform);
void drawText(vita2d_pgf *font, int x, int y,unsigned int color, float scale, const char *text);

/*******************************************************************************
Notes
*******************************************************************************/
/*---Display Notes
 * The coodinate system from vita2dlib goes by (x=0,y=0) being at the top left
 * x increases as you go right.
 * y increases as you go down.
 */
 
/*---Touch Notes
 *For some reason the touch data is twice that of what is drawn.
 *For this reason we have to test half rather than directly.
 */
 
/*---Collision notes
 * Check if the player is on top of a platform and give them a boost if they are
 * Ignore other collisions so that the player can go through the platforms.
 
 * collision can be done in this way:
 * Check if player.pos.y + player.height <= platform.y
 * check if atleast 1 pixel of the player lies above the platform width.
 * if both conditions are true, then the player is above on the platform.
 * 
 * now we need to consider if the player has landed on the platform.
 * most likely can be done by checking if the bottom of the player
 * is in a gap above the platform. A 5 pixel gap would be good. An issue with
 * this is that if between frames the player moves through this gap but does not
 * stop then no collision will occur. 
 * 
 * Action taken after this would usualy be to give the player a boost in
 * velocity to get to the next platform.
 */

/*******************************************************************************
Global variables - Sorry but it shouldn't really have an effect as they aren't
used in logic anywhere
*******************************************************************************/
vita2d_pgf *pgf; //Mostly used to print debug text throught the code.
/*******************************************************************************
main
*******************************************************************************/

int main(int argc, char *argv[]) 
{
	SceCtrlData pad;
	
	
	int boolDebug = 1;
	
	int platformNum = INITAL_PLAT_COUNT;
	
	vita2d_init();
	vita2d_set_clear_color(RGBA8(0x40, 0x40, 0x40, 0xFF));

	pgf = vita2d_load_default_pgf();
	
	srand(time(NULL));
	
	memset(&pad, 0, sizeof(pad)); // Control pad buttons
	
	//Touch data
	SceTouchData touchOld[SCE_TOUCH_PORT_MAX_NUM];
	SceTouchData touch[SCE_TOUCH_PORT_MAX_NUM];
	
	Platform_t platformArray[INITAL_PLAT_COUNT];
	generatePlatforms(platformArray, platformNum);
	
	Player_t player;
	player.pos.x = (PSVITA_SCREEN_WIDTH/2) - PLAYER_WIDTH/2;
	player.pos.y = 400;
	player.vel.x = 0;
	player.vel.y = 0;
	player.width = PLAYER_WIDTH;
	player.height = PLAYER_HEIGHT;
	player.colour = RED;
	
	while (1)
	{
		/*Exit when start is pressed*/
		sceCtrlPeekBufferPositive(0, &pad, 1);
		if (pad.buttons & SCE_CTRL_START)
			break;
		
		//R Trigger
		if (pad.buttons & SCE_CTRL_RTRIGGER)
		{
			boolDebug = !boolDebug;
		}
		
		if (pad.buttons & SCE_CTRL_UP)
		{
			player.vel.y -= 30;
		}
		if (pad.buttons & SCE_CTRL_DOWN)
		{
			player.vel.y += 30;
		}
			
		vita2d_start_drawing();
		vita2d_clear_screen();
		setUpTouchScreen();
		
		int port;
		for(port = 0; port < SCE_TOUCH_PORT_MAX_NUM; port++)
		{
			sceTouchPeek(port, &touch[port], 1);
		}
		
		/*-----^THIS STUFF MUST REMAIN AT THE TOP^-------*/
		int currentTop = 50;
		
		handleTouch(&player, touch);
		
		handleCollisionChecking(&player, platformArray, platformNum);
		
		handlePhysics(&player);
		
		drawPlayer(player);
		
		drawPlatforms(platformArray, platformNum);
		
		/*-------------DEBUG INFO-----------------*/
		if (boolDebug)
		{
			int port,i,j;
			char touchOutput[256] = "";
			char platOutput[256] = "";
			char playerOutput[256] = "";
			/* sample both back and front surfaces */
			for(port = 0; port < SCE_TOUCH_PORT_MAX_NUM; port++)
			{
				sprintf(touchOutput + strlen(touchOutput), "%s",((const char*[]){"FRONT:","BACK: "})[port]);
				/* print every touch coordinates on that surface */
				for(i = 0; i < touch[port].reportNum; i++)
				{
					sprintf(touchOutput + strlen(touchOutput), "%4i:%-4i ", touch[port].report[i].x,touch[port].report[i].y);
				}
				sprintf(touchOutput + strlen(touchOutput),"\n");				
				sprintf(touchOutput + strlen(touchOutput), "reportNum: %d \n", touch[port].reportNum);
			}
			
			for(j = 0;j < platformNum;j++)
			{
				sprintf(platOutput + strlen(platOutput), "plat %d: x: %0.2f, y: %0.2f \n", j, platformArray[j].pos.x, platformArray[j].pos.y);
			}
			sprintf(playerOutput + strlen(playerOutput),"Player Pos: x: %0.2f, y: %0.2f\n", player.pos.x, player.pos.y);
			sprintf(playerOutput + strlen(playerOutput),"Player Vel: x: %0.2f, y: %0.2f\n", player.vel.x, player.vel.y);
			
			
			drawText(pgf, 0, 250, GREEN , 1.0f, platOutput);	
			drawText(pgf, 0, 100, GREEN , 1.0f, touchOutput);		
			drawText(pgf, 0, 10, GREEN , 1.0f, playerOutput);
		}
		
		vita2d_end_drawing();
		vita2d_swap_buffers();
	}
	
	
	/*
	 * vita2d_fini() waits until the GPU has finished rendering,
	 * then we can free the assets freely.
	 */
	vita2d_fini();
	vita2d_free_pgf(pgf);
	
	
	sceKernelExitProcess(0);
	return 0;
}

void setUpTouchScreen()
{
	/* should use SCE_TOUCH_SAMPLING_STATE_START instead of 1 but old SDK have an invalid values */
	sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, 1);
	sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, 1);
	sceTouchEnableTouchForce(SCE_TOUCH_PORT_FRONT);
}

void drawPlayer(Player_t player)
{
	vita2d_draw_rectangle(player.pos.x, player.pos.y, player.width, player.height, player.colour);
}

void generatePlatforms(Platform_t platforms[], int numPlat)
{
	
	float i;
	for (i = 0.0 ;i < numPlat;i++)
	{
		
/*		if ((int)i % 4 == 0)*/
/*	    {*/
/*	        //This can be used in the future to select special platforms*/
/*	        int blackTile = rand() % 5;*/
/*	    }*/
	
		float row = floor(i/10.0);
		float x = rand() % PSVITA_SCREEN_WIDTH;
		float y = row + ((i/10.0) * PSVITA_SCREEN_HEIGHT);
		
		
		Platform_t newPlat;
		newPlat.pos.x = x;
		newPlat.pos.y = y;
		newPlat.width = PLATFORM_WIDTH;
		newPlat.height = PLATFORM_HEIGHT;
		newPlat.type = NORMAL;
		newPlat.colour = SKY_BLUE;
		platforms[(int)i] = newPlat;
	}
}

/*******************************************************************************
User input functions
*******************************************************************************/
void handleTouch(Player_t *player, SceTouchData touch[])
{
	int i,port;
	for(port = 0; port < SCE_TOUCH_PORT_MAX_NUM; port++)
	{
		/* print every touch coordinates on that surface */
		for(i = 0; i < touch[port].reportNum; i++)
		{
			if (touch[port].report[i].x < PSVITA_TOUCH_WIDTH / 2)
			{
				player->pos.x -= 3;
			}
			else
			{
				player->pos.x += 3;
			}
		}
	}
}

/*******************************************************************************
Physics related functions
*******************************************************************************/
void handlePhysics(Player_t *player)
{
	// Integrate to get Velocity
	player->vel.y += GRAVITY;
	// Integrate to get position
	player->pos.y += player->vel.y * TIME;
}

void handleCollisionChecking(Player_t *player, Platform_t platArray[], int platformLength)
{
	int i;
	for (i = 0;i < platformLength; i++)
	{
		int onPlatform = isOnPlatform(player->pos.x, player->pos.y, player->width, player->height,
									  platArray[i].pos.x, platArray[i].pos.y, platArray[i].width, platArray[i].height);
		if (onPlatform)
		{
			player->vel.y = -300; //Override any current velocity 
			platArray[i].colour = CLEARED_GREEN;
			char Out[256] = ""; 
			sprintf(Out + strlen(Out),"Colliding with plat: %d", i);
			drawText(pgf, 600, i*20, GREEN , 1.0f, Out);
		}
	}
}

int isOnPlatform(float playerX, float playerY, float playerWidth, float playerHeight, 
				 float platformX, float platformY, float platformWidth, float platformHeight)
{

	//check if atleast one corner is on the platform
	int cornerLeft = playerX;
	int cornerRight = playerX + playerWidth;
	//returns true if atleast one corner is on the platform.	
	
	if (testInRange(cornerLeft, platformX, platformX + platformWidth) ||
		testInRange(cornerRight, platformX, platformX + platformWidth))
	{
		//now it is above we need to check if it is landed on the platform
		return testInRange(playerY + playerHeight, platformY - PLATFORM_LANDING_SENSITIVITY, platformY + platformHeight);
	}
	
}

int testInRange(int numberToCheck, int bottom, int top)
{
 	return (numberToCheck >= bottom && numberToCheck <= top);
}

/*******************************************************************************
rendering functions
*******************************************************************************/

void drawPlatforms(Platform_t platforms[], int platformLength)
{
	int i;
	for (i = 0;i < platformLength;i++)
	{
		/*goes by: vita2d_draw_rectangle(int xPos, int yPos, int width, int height, RGBA8(R, G, B, A) colour);*/
		vita2d_draw_rectangle(platforms[i].pos.x, platforms[i].pos.y, platforms[i].width, platforms[i].height, platforms[i].colour);
		drawBorder(platforms[i]);		
	}
}

void drawBorder(Platform_t platform)
{
	//void vita2d_draw_line(float x0, float y0, float x1, float y1, unsigned int color)
	unsigned int colour = BLACK;
	vita2d_draw_line(platform.pos.x, platform.pos.y, platform.pos.x, platform.pos.y + platform.height, colour); //left border
	vita2d_draw_line(platform.pos.x, platform.pos.y + platform.height, platform.pos.x + platform.width, platform.pos.y + platform.height, colour);	//bottom border
	vita2d_draw_line(platform.pos.x + platform.width, platform.pos.y + platform.height, platform.pos.x + platform.width, platform.pos.y, colour); //right
	vita2d_draw_line(platform.pos.x + platform.width, platform.pos.y, platform.pos.x, platform.pos.y, colour);
	
}

void drawText(vita2d_pgf *font, int x, int y,unsigned int colour, float scale,const char *text)
{
	vita2d_pgf_draw_text(font, x, y, colour , scale, text);
}
