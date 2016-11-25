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
#define GREEN RGBA8(0,186,6,255)

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
SceTouchData pollTouchedTiles(Tile_t* tileArray, int tileArrayLengths);
void drawBorder(Tile_t tile);
void drawTiles(Tile_t tiles[], int tileLength);
void drawText(vita2d_pgf *font, int x, int y,unsigned int color, float scale,const char *text);

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
 *so all i do is compare to the old value to see if it's changed
 */

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
	srand(time(NULL));
	int blackTile = rand() % (4 + 1 - 0) + 0;
	int tileArrayLength = 0;
	Tile_t tileArray[16]; 
	float i;
	for (i=0.0;i<4;i++)
	{		
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
	
	
	memset(&pad, 0, sizeof(pad)); // Control pad buttons
	
	//Touch data
	SceTouchData touchOld;
	SceTouchData touch;
	
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
		
		if (pad.buttons & SCE_CTRL_LTRIGGER)
		{
			int i;
			for (i = 0;i < tileArrayLength;i++)
			{
				tileArray[i].position.y += 1;
			}
		}
			
		vita2d_start_drawing();
		vita2d_clear_screen();
		setUpTouchScreen();
		
		/*-----^THIS STUFF MUST REMAIN AT THE TOP^-------*/

			
		drawTiles(tileArray, tileArrayLength);
		
		
		/*touch code*/
		
		int i, port = SCE_TOUCH_PORT_FRONT;		
		//samplefront surfaces
		sceTouchPeek(port, &touch, 1);
		//print every touch coordinates on that surface
		for (i = 0; i < SCE_TOUCH_MAX_REPORT - 2; i++)
		{
			char strBuffer[50];
			sprintf(strBuffer,"x:%4i, y:%-4i ",touch.report[i].x,touch.report[i].y);
			drawText(pgf, 0, (i+1)*20, GREEN , 1.0f, strBuffer);
		}
		
		touch = pollTouchedTiles(tileArray, tileArrayLength);
		/*
		if ( (touch[SCE_TOUCH_PORT_FRONT].reportNum == 1)
		  && (touch_old[SCE_TOUCH_PORT_FRONT].reportNum == 1)
		  && (touch[SCE_TOUCH_PORT_FRONT].report[0].y >= 1000)
		  && (touch_old[SCE_TOUCH_PORT_FRONT].report[0].y < 1000))
		break;
		*/
		
		
		
		
		/*-------------DEBUG INFO-----------------*/
		if (boolDebug)
		{
			int i;
			for (i = 0;i < tileArrayLength;i++)
			{
				char strBuffer[50];
				sprintf(strBuffer, "Tile #%d is at position: (%0.1f,%0.1f), touched? %d", i, tileArray[i].position.x,tileArray[i].position.y,tileArray[i].touched);
				drawText(pgf, 200, (i+1)*20, GREEN , 1.0f, strBuffer);
			}		
			
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


SceTouchData pollTouchedTiles(Tile_t* tileArray, int tileArrayLength)
{
	//i'm really really sorry for this, how else was i supposed to loop through 2 arrays and test 2 conditions!?!?!?
	
	SceTouchData touch;
	sceTouchPeek(SCE_TOUCH_PORT_FRONT, &touch, 1);
	int i,j;
	Tile_t* tilep = tileArray;
	for (i = 0;i < tileArrayLength;i++, tilep++)
	{		
		for (j = 0; j < SCE_TOUCH_MAX_REPORT - 2; j++) // - 2 is because SCE_TOUCH_MAX_REPORT is 8 while it should really be 6 
		{
			int touchX = touch.report[j].x / 2; //factor pixel changes, (refer to notes)
			int touchY = touch.report[j].y / 2;
			if (touchX >= tilep->position.x && touchX <= tilep->position.x + tilep->width)
			{
				if (touchY >= tilep->position.y && touchY <= tilep->position.y + tilep->height)
				{
					//touch inside tile 
					tilep->beingTouched = 1;
					tilep->touched = 1;
				}
				else
				{
					tilep->beingTouched = 0;
				}
			}
			else
			{
				tilep->beingTouched = 0;
			}
		}
	}
	return touch;
}

Tile_t createNewTile(int xPos, int yPos, int width, int height, unsigned int colour)
{
	Tile_t tile;
	tile.position.x = xPos;
	tile.position.y = yPos;
	tile.width = width;
	tile.height = height;
	tile.colour = colour;
	tile.beingTouched = 0;
	tile.touched = 0;
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
