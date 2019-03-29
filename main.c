#include "address_map_arm.h"
#define KEYS_IRQ 73 	
#define IRQ_MODE 0b10010//18	
#define INT_DISABLE 0b11000000//192
#define INT_ENABLE 0b01000000
#define ENABLE 0x1		
#define SVC_MODE 0b10011
//Will testing git

volatile int pixel_buffer_start; // global variable

void config_KEYs();
void set_A9_IRQ_stack(void);
void config_GIC(void);
void enable_A9_interrupts(void);


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
    *(pixel_ctrl_ptr + 1) = 0xC0000000;
    pixel_buffer_start = *(pixel_ctrl_ptr + 1); // we draw on the back buffer


    while (1)
    {
        /* Erase any boxes and lines that were drawn in the last iteration */
        clear_screen();
		drawBox(x_Boat,y_Boat,width, height, color_Boat);
      
			
			if(x_Boat>=319-width || x_Boat<=0){
				dx_Boat = 0;
			}
			
			if(y_Boat>=239-height || y_Boat<=0){
				dy_Boat = 0;
			}	
			
			x_Boat = x_Boat+dx_Boat;
			y_Boat = y_Boat+dy_Boat;
		
		
        wait_for_vsync(pixelStatusPtr,pixel_ctrl_ptr); // swap front and back buffers on VGA vertical sync
        pixel_buffer_start = *(pixel_ctrl_ptr + 1); // new back buffer
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
*(KEY_ptr + 2) = 0x3; // enable interrupts for KEY[1]
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
void config_GIC(void)
{
int address; // used to calculate register addresses

/* configure the KEYs interrupts */
*((int *)0xFFFED108) = 0x00000300;
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
