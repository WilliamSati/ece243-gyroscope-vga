#include "address_map_arm.h"
#define KEYS_IRQ 73 	
#define IRQ_MODE 0b10010//18	
#define INT_DISABLE 0b11000000//192
#define INT_ENABLE 0b01000000
#define ENABLE 0x1		
#define SVC_MODE 0b10011
#include "math.h"

volatile int pixel_buffer_start; // global variable

void config_KEYs();
void set_A9_IRQ_stack(void);
void config_GIC(void);
void enable_A9_interrupts(void);
void manageKeyPress(volatile int* keyData);



void draw_line(int x0, int x1, int y0, int y1, short int color);
void clear_screen();
void wait_for_vsync(volatile int* pixelStatusPtr, volatile int* pixel_ctrl_ptr);
void drawBox(int xLeft, int yTop, int width, int height, short int color_Xs);

int dx_Boat;//default 0;
int dy_Boat;
volatile int key_dir = 0;
//320x240

int main(void){
	
set_A9_IRQ_stack(); // initialize the stack pointer for IRQ mode
config_GIC(); // configure the general interrupt controller
// interrupts
config_KEYs(); // configure pushbutton KEYs to generate interrupts
enable_A9_interrupts(); // enable interrupts
	
	volatile int * pixelStatusPtr = (int *)0xFF20302C;
	volatile int * front_buffer = (int *)0xFF203020;
	volatile int * back_buffer    = (int *)0xFF203024;
    // declare other variables(not shown)
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
	
	
	
	volatile int* keyData = 0xFF200050;
	//volatile int* switchData = 0x;
	int black = 0;
	int previous_x_Boat = x_Boat;
	int previous_y_Boat = y_Boat;
	
	
    while (1)
    {
       
		
		//poll the user's input to updsate dx_Boat and dy_Boat
		if(*keyData!=0){
			manageKeyPress(keyData);
		}
		
		//update previous boat_position before changing the boat position
		previous_x_Boat = x_Boat;
		previous_y_Boat = y_Boat;
		
		//make sure the updated position will be valid, then update the boat's position
		if(x_Boat + dx_Boat > 320-(width-1)){
			dx_Boat = 0;
			x_Boat = 320-(width-1);
		}
		else if(x_Boat + dx_Boat < 0){
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

void plot_pixel(int x, int y, short int line_color)
{
    *(short int *)(pixel_buffer_start + (y << 10) + (x << 1)) = line_color;
}

// code for subroutines (not shown)
void draw_line(int x0, int x1, int y0, int y1, short int color){
	int absy = y1-y0;
	int absx = x1-x0;
	if(absy < 0){
		absy = absy*(-1);
	}
	if(absx < 0){
		absx = absx*(-1);
	}

	int isSteep = absy > absx;
	int swap = 0;
	
	if (isSteep){
		swap = x0;
		x0 = y0;
		y0 = swap;
		
		swap = x1;
		x1 = y1;
		y1 = swap;
	}
	
	if (x1 < x0){
		swap = x0;
		x0 = x1;
		x1 = swap; 
		
		swap = y0;
		y0 = y1;
		y1 = swap;
	}

	int deltaX = x1 - x0;
	int deltaY = y1 - y0;
	int yStep = 1;
	
	if(deltaY < 0){
		deltaY = deltaY*(-1);
		yStep = -1;
	}
	
	int error = -(deltaX/2);
	
	int y = y0;
	int x = x0;
	for (;  x <= x1 ; x++ ){
		
		if(isSteep){
			plot_pixel(y,x,color);
		}else{
			plot_pixel(x,y,color);
		}
		
		error = error + deltaY;
		
		if(error >= 0){
			y = y + yStep;
			error = error - deltaX;
		}
	}
}

void clear_screen(){
	int x = 0;
	int y = 0;
	
	for( ; x < 320 ; x++){
		y = 0;
		for( ; y < 240 ; y++ ){
			plot_pixel(x,y,0);
		}
		
	}
}
void wait_for_vsync(volatile int* pixelStatusPtr, volatile int* pixel_ctrl_ptr){
	*pixel_ctrl_ptr = 1; //initializ the S bit
	while ((*pixelStatusPtr&1)!=0){//don't draw the next thing until the whole screen has been drawn
			;
		}
}

void drawBox(int xLeft, int yTop, int width, int height, short int color_Xs){
	int i = 0;
	for(; i<width ; i++){
		
		int j = 0;
		for(; j < height ; j++){
			plot_pixel(xLeft+i,yTop+j, color_Xs);
		}
		
	}
}
void manageKeyPress(volatile int* keyData){
	int keysPressed = *keyData;
	int arrayOfPressedKeys[4];
	
	int i = 0;
	
	for(; i<4 ; i++){
		arrayOfPressedKeys[i] = keysPressed&0b1;
		keysPressed/=2;
	}
	
	if(arrayOfPressedKeys[0]){
		dx_Boat+=1;
	}
	if(arrayOfPressedKeys[1]){
		dy_Boat-=1;
	}
	if(arrayOfPressedKeys[2]){
		dy_Boat+=1;
	}
	if(arrayOfPressedKeys[3]){
		dx_Boat-=1;
	}
		
}





void pushbutton_ISR(void);
// Define the IRQ exception handler
void __attribute__((interrupt)) __cs3_isr_irq(void)
{
// Read the ICCIAR from the processor interface
int address = MPCORE_GIC_CPUIF + ICCIAR;
int int_ID = *((int *)address);

if (int_ID == KEYS_IRQ) // check if interrupt is from the KEYs
pushbutton_ISR();
else
while (1)
; // if unexpected, then stay here
// Write to the End of Interrupt Register (ICCEOIR)

address = MPCORE_GIC_CPUIF + ICCEOIR;
*((int *)address) = int_ID;
return;
}




/***************************************************************************************
* Pushbutton - Interrupt Service Routine
*
* This routine toggles the key_dir variable from 0 <-> 1
****************************************************************************************/
void pushbutton_ISR(void)
{
volatile int * KEY_ptr = (int *)KEY_BASE;
int press;
press = *(KEY_ptr + 3); // read the pushbutton interrupt register
*(KEY_ptr + 3) = press; // Clear the interrupt



if (press == 0b1000)
	dx_Boat-=1;
	
else if (press == 0b0100)
	

dx_Boat+=1;
dy_Boat+=1;



key_dir ^= 1; // Toggle key_dir value
return;
}


// Define the remaining exception handlers
void __attribute__((interrupt)) __cs3_reset(void)
{
while (1)
;
}
void __attribute__((interrupt)) __cs3_isr_undef(void)
{
while (1)
;
}
void __attribute__((interrupt)) __cs3_isr_swi(void)
{
while (1)
;
}
void __attribute__((interrupt)) __cs3_isr_pabort(void)
{
while (1)
;
}
void __attribute__((interrupt)) __cs3_isr_dabort(void)
{
while (1)
;
}
void __attribute__((interrupt)) __cs3_isr_fiq(void)
{
while (1)
;
}


void config_KEYs()
{
volatile int * KEY_ptr = (int *)KEY_BASE; // pushbutton KEY address
*(KEY_ptr + 2) = 0xf; // enable interrupts for all keys
}
/*
* Initialize the banked stack pointer register for IRQ mode
*/
void set_A9_IRQ_stack(void)
{
int stack, mode;
stack = A9_ONCHIP_END - 7; // top of A9 onchip memory, aligned to 8 bytes
/* change processor to IRQ mode with interrupts disabled */
mode = INT_DISABLE | IRQ_MODE;
asm("msr cpsr, %[ps]" : : [ps] "r"(mode));
/* set banked stack pointer */
asm("mov sp, %[ps]" : : [ps] "r"(stack));
/* go back to SVC mode before executing subroutine return! */
mode = INT_DISABLE | SVC_MODE;
asm("msr cpsr, %[ps]" : : [ps] "r"(mode));
}
/*
* Turn on interrupts in the ARM processor
*/
void enable_A9_interrupts(void)
{
int status = SVC_MODE | INT_ENABLE;
asm("msr cpsr, %[ps]" : : [ps] "r"(status));
}
/*
* Configure the Generic Interrupt Controller (GIC)
*/
void config_GIC(void) //right now nothing is configured
{
int address; // used to calculate register addresses


/* configure the KEYs interrupts */
//*((int *)0xFFFED848) = 0x00000101;
// Set Interrupt Priority Mask Register (ICCPMR). Enable interrupts of all
// priorities
address = MPCORE_GIC_CPUIF + ICCPMR;
*((int *)address) = 0xFFFF;
// Set CPU Interface Control Register (ICCICR). Enable signaling of
// interrupts
address = MPCORE_GIC_CPUIF + ICCICR;
*((int *)address) = ENABLE;
// Configure the Distributor Control Register (ICDDCR) to send pending
// interrupts to CPUs
address = MPCORE_GIC_DIST + ICDDCR;
*((int *)address) = ENABLE;
}
