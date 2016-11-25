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
/**********************     Object size defines   *****************************/
/*for some reason the touch screen sizes are double*/
#define PSVITA_TOUCH_WIDTH 1920
#define PSVITA_TOUCH_HEIGHT 1088

#define PSVITA_SCREEN_WIDTH 960
#define PSVITA_SCREEN_HEIGHT 544

#define PLATFORM_WIDTH 100
#define PLATFORM_HEIGHT 10

#define PLAYER_WIDTH 50
#define PLAYER_HEIGHT 80

#define INITAL_PLAT_COUNT 10

/**********************   Physics Constants defines****************************/
#define TIME 1/40
#define GRAVITY 2


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


void handleCollisionChecking(Player_t, Platform_t playArray[], int platformLength);
int boundedBoxCollisionCheck(int x1, int y1, int width1, int height1, 
							 int x2, int y2, int width2, int width2);


void drawPlayer(Player_t player);
void drawPlatforms(Platform_t platforms[], int platformLength);
void drawBorder(Platform_t platform);
void drawText(vita2d_pgf *font, int x, int y,unsigned int color, float scale, const char *text);

/*******************************************************************************
Notes
*******************************************************************************/
/*---Touch Notes
 *For some reason the touch data is twice that of what is drawn.
 *For this reason we have to test half rather than directly.
 *
 *beingTouched is almost never able to be printed out as it is always 0
 *use Touched instead.
 *
 *There's no way to clear the values of touch when there is no touch happening
 *so all I do is compare to the old value to see if it's changed
 */

/*******************************************************************************
main
*******************************************************************************/

int main(int argc, char *argv[]) 
{
	SceCtrlData pad;
	vita2d_pgf *pgf;
	
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
	player.pos.y = 450;
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
		newPlat.colour = BLACK;
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

void handleCollisionChecking(Player_t *player, Platform_t playArray[], int platformLength)
{
	int i;
	for (i = 0;i < platformLength; i++)
	{
		int colliding = boundedBoxCollisionCheck(player->pos.x, player->pos.y, player->width, player->height,
												platArray[i].pos.x, platArray[i].pos.y, platArray[i].width, platArray[i].height)
		if (colliding)
		{
			
		}
	}
}

int boundedBoxCollisionCheck(int x1, int y1, int width1, int height1, 
							 int x2, int y2, int width2, int width2)
{
	return (x1 < x2 + width2 &&
		   x1 + width1 > x2 &&
		   y1 < y2 + height2 &&
		   height1 + y1 > y2);
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
