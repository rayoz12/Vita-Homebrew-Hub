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
#define PLATFORM_HEIGHT 15

#define PLAYER_WIDTH 50
#define PLAYER_HEIGHT 80

#define INITAL_PLAT_COUNT 8

/**********************   Physics Constants defines****************************/
#define TIME 1/40
#define GRAVITY 7

/*************************   Collision defines     ****************************/
#define PLATFORM_LANDING_SENSITIVITY 5 //The pixel gap at which collision with a platform is detected


/*******************************************************************************
Structs
*******************************************************************************/

struct Time
{
    int seconds;
    int minutes;
    int hours;
};

typedef struct Time Time_t;

/* TODO: why touch[port].report[i].force is always at 128 ? */

/*******************************************************************************
Function prototypes
*******************************************************************************/

void setUpTouchScreen();

void incrementTimer(Time_t *timer);

void clearTimer(Time_t *timer);

void timerToString(Time_t timer, char outString[]);

void drawText(vita2d_pgf *font, int x, int y,unsigned int colour, float scale,const char *text);
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
 
/*---Logic
 *Increment seconds then
 *Check if minutes/hours need to be incremented to.
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
	
	int boolPaused = 0;
	
	vita2d_init();
	vita2d_set_clear_color(RGBA8(0x40, 0x40, 0x40, 0xFF));

	pgf = vita2d_load_default_pgf();
	
	srand(time(NULL));
	
	memset(&pad, 0, sizeof(pad)); // Control pad buttons
	
	Time_t timer;
	timer.seconds = 0;
	timer.minutes = 0;
	timer.hours = 0;
	
	//Touch data
	SceTouchData touch[SCE_TOUCH_PORT_MAX_NUM];
	
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
		
		//L Trigger
		if (pad.buttons & SCE_CTRL_CROSS)
		{
			boolPaused = !boolPaused;
		}
		
			
		vita2d_start_drawing();
		vita2d_clear_screen();
		
		int port;
		for(port = 0; port < SCE_TOUCH_PORT_MAX_NUM; port++)
		{
			sceTouchPeek(port, &touch[port], 1);
		}
		
		/*-----^THIS STUFF MUST REMAIN AT THE TOP^-------*/
		
		//controls
		drawText(pgf, 0, 20, SKY_BLUE , 1.0f, "Press X to toggle timer, O to Pause, [] to Reset.");
		
		//Output
		char timerOutput[256];
		char debugOut[256] = "";
		timerToString(timer, timerOutput);
		sprintf(debugOut + strlen(debugOut), "Paused: %d\n", boolPaused);
		
		drawText(pgf, PSVITA_SCREEN_WIDTH/2 - 50, 250, WHITE , 2.0f, timerOutput);
		drawText(pgf, PSVITA_SCREEN_WIDTH/2 - 50, 250, WHITE , 2.0f, timerOutput);
		
		if (!boolPaused)
			incrementTimer(&timer);

		
		vita2d_end_drawing();
		vita2d_swap_buffers();
		
		if (!boolPaused)
			sceKernelDelayThread(999900); //Delay for nearly 1 second
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

/*******************************************************************************
setUpTouchScreen
Sets up the touch screen to detect touch on the front and back panels
inputs:
- none
outputs:
- none
*******************************************************************************/
void setUpTouchScreen()
{
	/* should use SCE_TOUCH_SAMPLING_STATE_START instead of 1 but old SDK have an invalid values */
	sceTouchSetSamplingState(SCE_TOUCH_PORT_FRONT, 1);
	sceTouchSetSamplingState(SCE_TOUCH_PORT_BACK, 1);
	sceTouchEnableTouchForce(SCE_TOUCH_PORT_FRONT);
}

void incrementTimer(Time_t *timer)
{
	if (++timer->seconds == 60)
	{
		timer->seconds = 0;
		if (++timer->minutes == 60)
		{
			timer->minutes = 0;
			++timer->hours;
		}
	}
}

void clearTimer(Time_t *timer)
{

}

void timerToString(Time_t timer, char outString[])
{
	char timerOutput[256] = "";
	sprintf(timerOutput + strlen(timerOutput), "Hour: Minute : second: \n");
	sprintf(timerOutput + strlen(timerOutput), "  %2d:%2d:%2d \n", timer.hours, timer.minutes, timer.seconds);
	strcpy(outString, timerOutput);	
}

void drawText(vita2d_pgf *font, int x, int y,unsigned int colour, float scale,const char *text)
{
	vita2d_pgf_draw_text(font, x, y, colour , scale, text);
}
