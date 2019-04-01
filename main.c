#include "constants.h"
#include "address_map_arm.h"
#include "math.h"
#include "drawingFunctions.h"
#include "externalVariables.h"
#include "boardSetupFunctions.h"
#include "manageUserInputFunctions.h"
#include "boatFunctions.h"
#include "stdbool.h" 
#include "stdlib.h"


//global variable definition (memory is allocated)
volatile int pixel_buffer_start; // global variable
double dx_Boat=0;//default 0;
double dy_Boat=0;

//320x240
void drawRing(int rippleRadius, int rippleCenter_x,int rippleCenter_y,short int ring_Color);
void updateRingPosition(int* rippleCenter_x, int* rippleCenter_y, int* rippleRadius, volatile int* switchData, bool* drawingRipple, int* furthestPoint,int* furthestVisibleDistance, bool* lastRipple);
void plotEightSymmetricPixels(int xCenter, int yCenter, int x, int y, short int ring_Color);
void updateFurthestPoint(int rippleCenter_x, int rippleCenter_y, int* furthestPoint);
bool screenContains(int x, int y);

int main(void){
	
set_A9_IRQ_stack(); // initialize the stack pointer for IRQ mode
config_GIC(); // configure the general interrupt controller
config_KEYs(); // configure pushbutton KEYs to generate interrupts
enable_A9_interrupts(); // enable interrupts
	
	volatile int * pixelStatusPtr = (int *)0xFF20302C;
	volatile int * front_buffer = (int *)0xFF203020;
	volatile int * back_buffer    = (int *)0xFF203024;

	int width = 8;
	int height = 8;
	short int color_Boat = 0xFD00;//orange
	short int ring_Color = 0x00FF;//Blueish
	int x_Boat=156;
	int y_Boat=114;
	

	
    /* set front pixel buffer to start of FPGA On-chip memory */
	
    *(back_buffer) = 0xC8000000; // first store the address in the 
                                        // back buffer
										
										
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync(pixelStatusPtr,front_buffer);
	
	
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *front_buffer;
	
	
    clear_screen(); // pixel_buffer_start points to the pixel buffer
	
    /* set back pixel buffer to start of SDRAM memory */
    *(back_buffer) = 0xC0000000;
    pixel_buffer_start = *(back_buffer); // we draw on the back buffer
	clear_screen(); // clear both buffers.
	
	
	
	volatile int* keyData = (int *) 0xFF200050;
	volatile int* switchData = (int *) SW_BASE;
	
	int black = 0;
	int previous_x_Boat = x_Boat;
	int previous_y_Boat = y_Boat;
	int previousRippleRadius = 1;
	int rippleRadius = 1;
	int rippleCenter_x;
	int rippleCenter_y;
	bool drawingRipple = 0;
	bool lastRipple = 0;
	int furthestPoint[2];
	int furthestVisibleDistance = 0;
    while (1)
    {
		//update previous boat_position and ring_position before changing the boat position
		previous_x_Boat = x_Boat;
		previous_y_Boat = y_Boat;
		previousRippleRadius = rippleRadius;
		
		
		//make sure the updated position will be valid, then update the boat's position
		updateBoatPositionAndSpeed(&x_Boat,&y_Boat,width,height,keyData);
		updateRingPosition(&rippleCenter_x, &rippleCenter_y,&rippleRadius, switchData, &drawingRipple, furthestPoint, &furthestVisibleDistance, &lastRipple);
		
		//now that the new position is valid, draw the new box on the back_buffer
		drawBox(x_Boat,y_Boat,width, height, color_Boat);
		if(drawingRipple)
			drawRing(rippleRadius, rippleCenter_x,rippleCenter_y,ring_Color);

		//now that everything is loaded properly, wait for the dma to switch front and back buffer addresses.
		wait_for_vsync(pixelStatusPtr,front_buffer); // swaps front and back buffers on VGA vertical sync
		pixel_buffer_start = *(back_buffer); // new back buffer
		
		/* Erase any boxes and lines that were drawn in the last iteration */
		drawBox(previous_x_Boat,previous_y_Boat,width, height, black);
		if(drawingRipple || lastRipple){
			drawRing(previousRippleRadius, rippleCenter_x, rippleCenter_y, black);
			lastRipple=0;
		}
    }
}

void updateRingPosition(int* rippleCenter_x, int* rippleCenter_y, int* rippleRadius, volatile int* switchData, bool* drawingRipple, int* furthestPoint,int* furthestVisibleDistance,bool* lastRipple){
	if(!*drawingRipple){
		if((*switchData & 1) == 0){ //check switch 1. If it's on, make a ripple, if it's not, don't make a ripple.
			;
		}else{
			*drawingRipple = 1;
			*rippleCenter_x = rand()%320;
			*rippleCenter_y = rand()%240;
			*rippleRadius = 0;
			
			*furthestVisibleDistance = 400;
		}
		return;		
	}
	
	if(*rippleRadius > *furthestVisibleDistance){
		*drawingRipple = 0;
		*lastRipple = 1;
	}else{
		*rippleRadius+=1;
	}
	
}
void updateFurthestPoint(int rippleCenter_x, int rippleCenter_y, int* furthestPoint){
	if(rippleCenter_x > 159.5 && rippleCenter_y > 119.5){
		furthestPoint[0] = 0;
		furthestPoint[1] = 0;
	}
	if(rippleCenter_x > 159.5 && rippleCenter_y < 119.5){
		furthestPoint[0] = 0;
		furthestPoint[1] = 239;
	}
	if(rippleCenter_x < 159.5 && rippleCenter_y > 119.5){
		furthestPoint[0] = 319;
		furthestPoint[1] = 0;
	}
	if(rippleCenter_x < 159.5 && rippleCenter_y < 119.5){
		furthestPoint[0] = 319;
		furthestPoint[1] = 239;
	}
}

void drawRing(int rippleRadius, int rippleCenter_x,int rippleCenter_y,short int ring_Color){
	//the following is Bresenham's algorithm.
	
	int x = 0, y = rippleRadius;
	int d = 3 - 2 * rippleRadius;
	plotEightSymmetricPixels(rippleCenter_x, rippleCenter_y, x, y, ring_Color);
	while(y >= x)
	{
		x+=1;
		if(d > 0)
		{
			y--;
			d = d + 4 * (x - y) + 10;
		}
		else{
			d = d + 4 * x + 6;
		}
	plotEightSymmetricPixels(rippleCenter_x,rippleCenter_y,x,y,ring_Color);
	}
}

void plotEightSymmetricPixels(int xCenter, int yCenter, int x, int y, short int ring_Color){
	
	if(screenContains(xCenter+x,yCenter+y))
		plot_pixel(xCenter+x,yCenter+y,ring_Color);
	
	if(screenContains(xCenter-x,yCenter+y))
		plot_pixel(xCenter-x,yCenter+y,ring_Color);
	
	if(screenContains(xCenter+x,yCenter-y))
		plot_pixel(xCenter+x,yCenter-y,ring_Color);
	
	if(screenContains(xCenter-x,yCenter-y))
		plot_pixel(xCenter-x,yCenter-y,ring_Color);
	
	if(screenContains(xCenter+y,yCenter+x))
		plot_pixel(xCenter+y,yCenter+x,ring_Color);
	
	if(screenContains(xCenter-y,yCenter+x))
		plot_pixel(xCenter-y,yCenter+x,ring_Color);
	
	if(screenContains(xCenter+y,yCenter-x))
		plot_pixel(xCenter+y,yCenter-x,ring_Color);
	
	if(screenContains(xCenter-y,yCenter-x))
		plot_pixel(xCenter-y,yCenter-x,ring_Color);
}

bool screenContains(int x, int y){
	if(x < 320 && x >= 0 && y < 240 && y >=0){
		return true;
	}else{
		return false;
	}
}



