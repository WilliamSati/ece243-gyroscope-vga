
#include "address_map_arm.h"
#include "math.h"
#include "drawingFunctions.h"
#include "externalVariables.h"
#include "boardSetupFunctions.h"
#include "manageUserInputFunctions.h"
#include "boatFunctions.h"
#include "stdbool.h" 
#include "stdlib.h"
#include "ringFunctions.h"

volatile int pixel_buffer_start; // global variable
int dx_Boat;//default 0;
int dy_Boat;
volatile int key_dir = 0;
//320x240

/*
void drawRing(int rippleRadius, int rippleCenter_x,int rippleCenter_y,short int ring_Color);
void updateRingPosition(int* rippleCenter_x, int* rippleCenter_y, int* rippleRadius, volatile int* switchData, bool* drawingRipple, int* furthestPoint,int* furthestVisibleDistance, bool* lastRipple);
void plotEightSymmetricPixels(int xCenter, int yCenter, int x, int y, short int ring_Color);
void updateFurthestPoint(int rippleCenter_x, int rippleCenter_y, int* furthestPoint);
bool screenContains(int x, int y);
*/

int main(void){
	
set_A9_IRQ_stack(); // initialize the stack pointer for IRQ mode
config_GIC(); // configure the general interrupt controller
// interrupts
config_KEYs(); // configure pushbutton KEYs to generate interrupts
enable_A9_interrupts(); // enable interrupts
	
	volatile int * pixelStatusPtr = (int *)0xFF20302C;
	volatile int * pixel_ctrl_ptr = (int *)0xFF203020;
    // declare other variables(not shown)
	int width = 8;
	int height = 8;
	short int color_Boat = 0xFD00;//orange
	int x_Boat=156;
	int y_Boat=116;

	

	
    /* set front pixel buffer to start of FPGA On-chip memory */
	
    *(pixel_ctrl_ptr + 1) = 0xC8000000; // first store the address in the 
                                        // back buffer
										
										
    /* now, swap the front/back buffers, to set the front buffer location */
    wait_for_vsync(pixelStatusPtr,pixel_ctrl_ptr);
	
	
    /* initialize a pointer to the pixel buffer, used by drawing functions */
    pixel_buffer_start = *pixel_ctrl_ptr;
	
	
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
	
	int previousRippleRadius[100];
	int rippleRadius[100];
	int numRipples = 0;
	
	int rippleCenter_x[100];
	int rippleCenter_y[100];
	bool drawingRipple[100];
	bool lastRipple[100];
	int furthestPoint[2]; //not currently used
	int furthestVisibleDistance = 0;//currently set to 400 always
	
	
	int i = 0; //iterator;
	
	
    while (1)
    {
		//update previous boat_position and ring_position before changing the boat position
		previous_x_Boat = x_Boat;
		previous_y_Boat = y_Boat;
		
		if(numRipples==0){
			;
		}else{
			i = 0;
			for(; i<numRipples ; i++)
				previousRippleRadius[i] = rippleRadius[i];
		}
		
		//make sure the updated position will be valid, then update the boat's position
		updateBoatPositionAndSpeed(&x_Boat,&y_Boat,width,height,keyData);
		//updates ALL of the rings
		updateRingPosition(rippleCenter_x, rippleCenter_y, rippleRadius, switchData, drawingRipple, furthestPoint, furthestVisibleDistance, lastRipple, &numRipples);
		
		//now that the new position is valid, draw the new box on the back_buffer
		drawBox(x_Boat,y_Boat,width, height, color_Boat);
		
		i = 0;
		for(; i<numRipples; i++){
			if(drawingRipple[i])
				drawRing(rippleRadius[i], rippleCenter_x[i],rippleCenter_y[i],ring_Color);
		}

		//now that everything is loaded properly, wait for the dma to switch front and back buffer addresses.
		wait_for_vsync(pixelStatusPtr,front_buffer); // swaps front and back buffers on VGA vertical sync
		pixel_buffer_start = *(back_buffer); // new back buffer
		
		/* Erase any boxes and lines that were drawn in the last iteration */
		drawBox(previous_x_Boat,previous_y_Boat,width, height, black);
		
		i=0;
		for(; i<numRipples; i++){
			if(drawingRipple[i] || lastRipple[i]){
				drawRing(rippleRadius[i], rippleCenter_x[i],rippleCenter_y[i],black);
				
				if(lastRipple[i]){
					int j = i;
					for(; j<(numRipples-1) ; j++){
						rippleCenter_x[j] = rippleCenter_x[j+1];
						rippleCenter_y[j] = rippleCenter_y[j+1];
						drawingRipple[j] = drawingRipple[j+1];
						lastRipple[j] = drawingRipple[j+1];
					}
						
					numRipples -=1 ;
				}
				
				
				lastRipple[i]=0;
			}
		}
    }
}







//not needed right now
/*
void updateFurthestPoint(int rippleCenter_x, int rippleCenter_y, int* furthestPoint){
	if(rippleCenter_x > 159.5 && rippleCenter_y > 119.5){
		furthestPoint[0] = 0;
		furthestPoint[1] = 0;
	}
}
*/
