/*******************************************************************************
List of header files
*******************************************************************************/
#include <psp2/kernel/processmgr.h>
#include <psp2/touch.h>
#include <psp2/ctrl.h>

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <stdlib.h>


#include <vita2d.h>

/*******************************************************************************
Defines
*******************************************************************************/
#define WHITE RGBA8(255, 255, 255, 255)
#define BLACK RGBA8(0, 0, 0, 255)
#define GREEN RGBA8(0,255,0,255)

#define PSVITA_SCREEN_WIDTH 960
#define PSVITA_SCREEN_HEIGHT 544

#define TILE_WIDTH 240
#define TILE_HEIGHT 181

/*******************************************************************************
Structs for tiles
*******************************************************************************/
struct Position
{
    float x;
    float y;
};


struct Tile
{
    struct Position position;
    int width;
    int height;
    unsigned int colour;//RGBA8() return type
    int beingTouched; //Bool -- if tile is currently being touched
    int touched; //Bool -- If the tile has been touched, used to check if gameover or not.
};

typedef struct Tile Tile_t;
 

/* TODO: why touch[port].report[i].force is always at 128 ? */

/*******************************************************************************
Function prototypes
*******************************************************************************/

void setUpTouchScreen();
Tile_t createNewTile(int xPos, int yPos, int width, int height, unsigned int colour);
void drawBorder(Tile_t tile);
void drawTiles(Tile_t tiles[], int tileLength);
void drawText(vita2d_pgf *font, int x, int y,unsigned int color, float scale,const char *text);


/*******************************************************************************
main
*******************************************************************************/

int main(int argc, char *argv[]) 
{
	SceCtrlData pad;
	vita2d_pgf *pgf;
	
	int boolDebug = 1;
	
	vita2d_init();
	vita2d_set_clear_color(RGBA8(0x40, 0x40, 0x40, 0xFF));

	pgf = vita2d_load_default_pgf();
	
	
	//set up tile array.
	//Tile notes:
	//12 Tiles total onscreen:
	//4 Horizontal, 3 Vertial
	int blackTile = rand() % (4 + 1 - 0) + 0;
	int tileArrayLength = 0;
	Tile_t tileArray[16]; 
	float i;
	for (i=0.0;i<4;i++)
	{
		srand(time(NULL));
		rand() % (4 + 1 - 0) + 0;		
		float x = (i/4) * PSVITA_SCREEN_WIDTH;
		float y = (2/3) * PSVITA_SCREEN_HEIGHT;
		//lat arg check if this is the black tile and sets if it is.
		tileArray[(int)i] = createNewTile(x,y,TILE_WIDTH,TILE_HEIGHT,(i==(int)blackTile) ? BLACK:WHITE);
	}
	//tileArray[0] = createNewTile(0 * PSVITA_SCREEN_WIDTH,2/3 * PSVITA_SCREEN_HEIGHT,TILE_WIDTH,TILE_HEIGHT,WHITE);
	//tileArray[1] = createNewTile((1/4) * PSVITA_SCREEN_WIDTH,2/3 * PSVITA_SCREEN_HEIGHT,TILE_WIDTH,TILE_HEIGHT,BLACK);
	//tileArray[2] = createNewTile((2/4) * PSVITA_SCREEN_WIDTH,2/3 * PSVITA_SCREEN_HEIGHT,TILE_WIDTH,TILE_HEIGHT,WHITE);
	//tileArray[3] = createNewTile((3/4) * PSVITA_SCREEN_WIDTH,2/3 * PSVITA_SCREEN_HEIGHT,TILE_WIDTH,TILE_HEIGHT,WHITE);
	tileArrayLength = 4;
	
	
	memset(&pad, 0, sizeof(pad));
	
	SceTouchData touch_old[SCE_TOUCH_PORT_MAX_NUM];
	SceTouchData touch[SCE_TOUCH_PORT_MAX_NUM];
	while (1)
	{
		/*Exit when start is pressed*/
		sceCtrlPeekBufferPositive(0, &pad, 1);
		if (pad.buttons & SCE_CTRL_START)
			break;
		//Right button
		if (pad.buttons & SCE_CTRL_RIGHT)
		{
			//increase width of tiles
			int i;
			for (i = 0;i < tileArrayLength;i++)
			{
				tileArray[i].position.x += 10.00;
			}
		}
		//left button
		if (pad.buttons & SCE_CTRL_LEFT)
		{
			//increase width of tiles
			int i;
			for (i = 0;i < tileArrayLength;i++)
			{
				tileArray[i].position.x -= 10.00;
			}
		}
		
		//Right button
		if (pad.buttons & SCE_CTRL_UP)
		{
			//increase width of tiles
			int i;
			for (i = 0;i < tileArrayLength;i++)
			{
				tileArray[i].position.y -= 10.00;
			}
		}
		//left button
		if (pad.buttons & SCE_CTRL_DOWN)
		{
			//increase width of tiles
			int i;
			for (i = 0;i < tileArrayLength;i++)
			{
				tileArray[i].position.y += 10.00;
			}
		}
		
		//R Trigger
		if (pad.buttons & SCE_CTRL_RTRIGGER)
		{
			boolDebug = !boolDebug;
		}
			
		vita2d_start_drawing();
		vita2d_clear_screen();
		setUpTouchScreen();
		
		/*-----^THIS STUFF MUST REMAIN AT THE TOP^-------*/

			
		drawTiles(tileArray, tileArrayLength);
		
		
		/*-------------DEBUG INFO-----------------*/
		if (boolDebug)
		{
			int i;
			for (i = 0;i < tileArrayLength;i++)
			{
				char strBuffer[50];
				sprintf(strBuffer, "Tile #%d is at position: (%f,%f)", i, tileArray[i].position.x,tileArray[i].position.y);
				drawText(pgf, 400, (i+1)*20, GREEN , 1.0f, strBuffer);
			}
			
			
		}

		

		
		
		
		
		
		/*touch code*/
		/*
		memcpy(touch_old, touch, sizeof(touch_old));
		int port,i;		
		//sample both back and front surfaces
		for(port = 0; port < SCE_TOUCH_PORT_MAX_NUM; port++){
			sceTouchPeek(port, &touch[port], 1);
			//print every touch coordinates on that surface
			for(i = 0; i < SCE_TOUCH_MAX_REPORT; i++)
				printf("\e[9%im%4i:%-4i ", (i < touch[port].reportNum)? 7:0,
				       touch[port].report[i].x,touch[port].report[i].y);
			printf("\n");
		}

		if ( (touch[SCE_TOUCH_PORT_FRONT].reportNum == 1)
		  && (touch_old[SCE_TOUCH_PORT_FRONT].reportNum == 1)
		  && (touch[SCE_TOUCH_PORT_FRONT].report[0].y >= 1000)
		  && (touch_old[SCE_TOUCH_PORT_FRONT].report[0].y < 1000))
		break;
		*/
		
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
	sceTouchEnableTouchForce(SCE_TOUCH_PORT_BACK);
}

Tile_t createNewTile(int xPos, int yPos, int width, int height, unsigned int colour)
{
	Tile_t tile;
	tile.position.x = xPos;
	tile.position.y = yPos;
	tile.width = width;
	tile.height = height;
	tile.colour = colour;
	
	return tile;
}


void drawTiles(Tile_t tiles[], int tileLength)
{
	int i;
	for (i = 0;i < tileLength;i++)
	{
		/*goes by: vita2d_draw_rectangle(int xPos, int yPos, int width, int height, RGBA8(R, G, B, A) colour);*/
		vita2d_draw_rectangle(tiles[i].position.x, tiles[i].position.y, tiles[i].width, tiles[i].height, tiles[i].colour);
		drawBorder(tiles[i]);		
	}
}

void drawBorder(Tile_t tile)
{
	//void vita2d_draw_line(float x0, float y0, float x1, float y1, unsigned int color)
	unsigned int colour = (tile.colour==WHITE) ? BLACK:WHITE;
	vita2d_draw_line(tile.position.x, tile.position.y, tile.position.x, tile.position.y + tile.height, colour); //left border
	vita2d_draw_line(tile.position.x, tile.position.y + tile.height, tile.position.x + tile.width, tile.position.y + tile.height, colour);	//bottom border
	vita2d_draw_line(tile.position.x + tile.width, tile.position.y + tile.height, tile.position.x + tile.width, tile.position.y, colour); //right
	vita2d_draw_line(tile.position.x + tile.width, tile.position.y, tile.position.x, tile.position.y, colour);
	
}

void drawText(vita2d_pgf *font, int x, int y,unsigned int colour, float scale,const char *text)
{
	vita2d_pgf_draw_text(font, x, y, colour , scale, text);
}
