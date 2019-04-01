#include "constants.h"
#include "address_map_arm.h"
#include "math.h"
#include "drawingFunctions.h"
#include "externalVariables.h"
#include "boardSetupFunctions.h"
#include "manageUserInputFunctions.h"



//global variable definition (memory is allocated)
volatile int pixel_buffer_start; // global variable
double dx_Boat=0;//default 0;
double dy_Boat=0;

//320x240

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
	
	int black = 0;
	int previous_x_Boat = x_Boat;
	int previous_y_Boat = y_Boat;
	
	
    while (1)
    {
       
		//poll the user's input to updsate dx_Boat and dy_Boat
		if(*keyData!=0){
			manageKeyPress(keyData);
		}
		
		//account for the drag force of the water
		if(dx_Boat>0){
			dx_Boat -= pow(dx_Boat,2)/75+0.01;
		}
		if(dx_Boat<0){
			dx_Boat += pow(dx_Boat,2)/75+0.01;
		}
		if(dy_Boat>0){
			dy_Boat -= pow(dy_Boat,2)/75+0.01;
		}
		if(dy_Boat<0){
			dy_Boat += pow(dy_Boat,2)/75+0.01;
		}
		
		//update previous boat_position before changing the boat position
		previous_x_Boat = x_Boat;
		previous_y_Boat = y_Boat;
		
		//make sure the updated position will be valid, then update the boat's position
		if(x_Boat + (int)dx_Boat > 320-(width-1)){
			dx_Boat = 0;
			x_Boat = 320-(width-1);
		}
		else if(x_Boat + (int)dx_Boat < 0){
			dx_Boat = 0;
			x_Boat = 0;
		}
		else{
			x_Boat = x_Boat + dx_Boat;
		}
		
		if(y_Boat + dy_Boat > 240-(height-1)){
			dy_Boat = 0;
			y_Boat = 240-(height-1);
		} else if(y_Boat + dy_Boat < 0){
			dy_Boat = 0;
			y_Boat = 0;
		} else{
			y_Boat = y_Boat + dy_Boat;
		}
		
		//now that the new position is valid, draw the new box on the back_buffer
		drawBox(x_Boat,y_Boat,width, height, color_Boat);
		

		//now that everything is loaded properly, wait for the dma to switch front and back buffer addresses.
		wait_for_vsync(pixelStatusPtr,front_buffer); // swaps front and back buffers on VGA vertical sync
		pixel_buffer_start = *(back_buffer); // new back buffer
		
		/* Erase any boxes and lines that were drawn in the last iteration */
		drawBox(previous_x_Boat,previous_y_Boat,width, height, black);
    }
}








