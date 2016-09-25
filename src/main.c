#include <psp2/kernel/processmgr.h>
#include <psp2/touch.h>
#include <psp2/ctrl.h>

#include <stdio.h>
#include <string.h>


#include <vita2d.h>

#define white RGBA8(255, 255, 255, 255)
#define black RGBA8(0, 0, 0, 255)


struct Position
{
    int x;
    int y;
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
void drawTiles(Tile_t[] tiles, int tileLength);


/*******************************************************************************
main
*******************************************************************************/

int main(int argc, char *argv[]) 
{
	SceCtrlData pad;
	vita2d_pgf *pgf;
	float rad = 0.0f;
	
	vita2d_init();
	vita2d_set_clear_color(RGBA8(0x40, 0x40, 0x40, 0xFF));

	pgf = vita2d_load_default_pgf();
	
	
	//set up tile array.
	int tileArrayLength = 0;
	Tile_t tileArray[16]; 
	tileArray[0] = createNewTile(500,20,50,100,white);
	tileArray[1] = createNewTile(600,130,50,100,black);
	tileArrayLength += 2;
	
	
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
				tileArray[i].width += 10;
			}
		}
		//left buuton
		if (pad.buttons & SCE_CTRL_LEFT)
		{
			//increase width of tiles
			int i;
			for (i = 0;i < tileArrayLength;i++)
			{
				tileArray[i].width -= 10;
			}
		}
			
		vita2d_start_drawing();
		vita2d_clear_screen();
		
		/*-----^THIS STUFF MUST REMAIN AT THE TOP^-------*/

			
		drawTiles(tileArray, tileArrayLength);

		

		
		/*setUpTouchScreen();*/
		
		
		
		
		
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


void drawTiles(Tile_t[] tiles, int tileLength)
{
	int i;
	for (i = 0;i < tileLength;i++)
	{
		/*goes by: vita2d_draw_rectangle(int xPos, int yPos, int width, int height, RGBA8(R, G, B, A) colour);*/
		vita2d_draw_rectangle(tiles[i].position.x, tiles[i].position.y, tiles[i].width, tiles[i].height, tiles[i].colour);
	}
}
