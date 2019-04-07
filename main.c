
#include "stdbool.h" 
#include "stdlib.h"
#include "math.h"
#include "address_map_arm.h"
#include "graphics_animation.h"
#include "graphics_util.h"
#include "board_init.h"
#include "gyro.h"

// global pointers
volatile int pixel_buffer_start;
volatile int key_dir = 0;

double dx_Boat = 0;//default 0;
double dy_Boat = 0;
int globalRingCounter = 0;

int main(void){
	set_A9_IRQ_stack(); // initialize the stack pointer for IRQ mode
	config_GIC(); // configure the general interrupt controller
	config_KEYs(); // configure pushbutton KEYs to generate interrupts
	enable_A9_interrupts(); // enable interrupts
	
	volatile int * pixelStatusPtr = (int *)0xFF20302C;
	volatile int * front_buffer = (int *)0xFF203020;
	volatile int * back_buffer  = (int *)0xFF203024;
	volatile int * led_ptr = (int *) LEDR_BASE;
	
    // declare other variables(not shown)
	int width = 8;
	int height = 8;
	short int color_Boat = 0xFD00;//orange
	short int ring_Color = 0x00FF;//blue
	int x_Boat=156;
	int y_Boat=116;

    /* set front pixel buffer to start of FPGA On-chip memory */
    *(back_buffer) = 0xC8000000; // first store the address in the back buffer
																		
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
	int numRipples = 0;
	
	int previousRippleRadius[1000];
	int rippleRadius[1000];
	int rippleCenter_x[1000];
	int rippleCenter_y[1000];
	bool lastRipple[1000];
	int furthestVisibleDistance = 100;//currently set to 400 always
		
	int i = 0; //iterator;
	
	// initialize gyroscope
	init_gyro();
	int gyro_x_rate = get_x_angular_rate();
	int gyro_y_rate = get_y_angular_rate();

	// event loop
    while (1)
    {	
    	// read output from gyroscope
		gyro_x_rate = get_x_angular_rate();
		gyro_y_rate = get_y_angular_rate();
		*led_ptr = gyro_y_rate >> 6;

		//update previous boat_position and ring_position before changing the boat position
		previous_x_Boat = x_Boat;
		previous_y_Boat = y_Boat;
		if((int)dx_Boat || (int)dy_Boat){
			globalRingCounter+=1; //is reset by updateRingPosition
		}
		
		i=0;
		for(; i <numRipples ; i++){
			previousRippleRadius[i] = rippleRadius[i];
		}
		
		//make sure the updated position will be valid, then update the boat's position
		updateBoatPositionAndSpeed(&x_Boat,&y_Boat,width,height,keyData, gyro_x_rate, gyro_y_rate);
		//updates ALL of the rings
		updateRingPosition(rippleCenter_x, rippleCenter_y, rippleRadius, switchData, furthestVisibleDistance, lastRipple, &numRipples, previousRippleRadius, x_Boat, y_Boat);
		//now that the new position is valid, draw the new box on the back_buffer
		drawBox(x_Boat,y_Boat,width, height, color_Boat);
		
		i = 0;
		for(; i<numRipples; i++){
			if(!lastRipple[i])
				drawRing(rippleRadius[i], rippleCenter_x[i],rippleCenter_y[i],ring_Color);
		}

		//now that everything is loaded properly, wait for the dma to switch front and back buffer addresses.
		wait_for_vsync(pixelStatusPtr,front_buffer); // swaps front and back buffers on VGA vertical sync
		pixel_buffer_start = *(back_buffer); // new back buffer
		
		/* Erase any boxes and lines that were drawn in the last iteration */
		drawBox(previous_x_Boat,previous_y_Boat,width, height, black);
		
		
		//erase all the rings
		i=0;
		for(; i < numRipples; i++){
				drawRing(previousRippleRadius[i], rippleCenter_x[i],rippleCenter_y[i],black);
		}
		
		//delete the ripples that are past the 
		i=0;
		for(; i < numRipples; i++){
				if(lastRipple[i]){
					int j = i;
					for(; j<(numRipples-1) ; j++){
						rippleCenter_x[j] = rippleCenter_x[j+1];
						rippleCenter_y[j] = rippleCenter_y[j+1];
						lastRipple[j] = lastRipple[j+1];
						previousRippleRadius[j] = previousRippleRadius[j+1];
						rippleRadius[j] = rippleRadius[j+1];
					}
					numRipples -=1 ;
				}
		}
	}  
}