
/*
 * Implementation of SPI communication between L3GD20 gyroscope and ARM.
 */

#include "gyro.h"
#include "address_map_arm.h"

volatile int* jp1_gpio_data_ptr = (int *) JP1_BASE;
volatile int* jp1_gpio_dir_ptr = (int *) (JP1_BASE + 4);

/*
 * Implementation of SPI for the L3GD20. Simple read and write to registers
 */

// function that reads register from a specified address on the L3GD20. Protocol specified on data sheet
int read_register( int address ){
	int register_val = 0x00;

	// drive clock high before CS is driven low
	clock_high(); delay();
    
	// drive CS pin low to start transmission
	*jp1_gpio_data_ptr &= ~(1 << CS); delay();
    
	// drive RW to 1 for register read
	SDI_high(); delay();   
	clock_low(); delay();  
	clock_high(); delay();

	// drive MS to 0, no auto-incremented reading
	SDI_low(); delay();  
	clock_low(); delay();   
	clock_high(); delay();
    
	// send address bits, 6 bits
    int i=5;
	for(; i>=0; i--){
		if((address & (1 << i)) != 0){
			SDI_high(); delay();
		}
		else {
			SDI_low(); delay();
		}
		clock_low(); delay();
		clock_high(); delay();
	}

	// recieve MSB to LSB through the SDO
    i=7;
	for(; i>=0; i--){
		clock_low(); delay();
		if((*jp1_gpio_data_ptr & (1 << SDO)) != 0){
			register_val |= (1 << i); delay();
		}
		else {
			register_val &= ~(1 << i); delay();
		}
		clock_high(); delay();
	}

	// finish transmission, drive CS high
	*jp1_gpio_data_ptr |= (1 << CS); delay();
    
    return register_val;
}

// function that writes to a register on the L3GD20
void write_register( int address, int data ){

	// drive clock high before CS is driven low
	clock_high(); delay();

	// drive CS pin low to start transmission
	*jp1_gpio_data_ptr &= ~(1 << CS);

	// drive RW to 0 for register write
	SDI_low(); delay();
    clock_low(); delay();
    clock_high(); delay();
    
	// drive MS to 0, no auto-incremented reading
	SDI_low(); delay();
    clock_low(); delay();
    clock_high(); delay();

	// send address bits, 6 bits
	int i=5;
    for(; i>=0; i--){
		// extract bits from address
		if((address & (1 << i)) != 0){
			SDI_high(); delay();
		}
		else {
			SDI_low(); delay();
		}
		clock_low(); delay();
		clock_high(); delay();
	}

	// recieve MSB to LSB through the SDO
	i=7;
    for(; i>=0; i--){
		// extract bits from data
		if((data & (1 << i)) != 0){
			SDI_high(); delay();
		}
		else {
			SDI_low(); delay();
		}
		clock_low(); delay();
		clock_high(); delay();
	}

	// finish transmission, drive CS high
	*jp1_gpio_data_ptr |= (1 << CS);
}

// convinience functions for toggling inputs to gyro
void clock_low( void ){
	// drive clock low
	*jp1_gpio_data_ptr &= ~(1 << SPC);
}
void clock_high( void ){
	// drive clock high
	*jp1_gpio_data_ptr |= (1 << SPC);
}
void SDI_low( void ){
	*jp1_gpio_data_ptr &= ~(1 << SDI);
}
void SDI_high( void ){
	*jp1_gpio_data_ptr |= (1 << SDI);
}

// delay functions for debugging and timing
void delay( void ){
	// delay count of 0.001 second
    int delay_count = 200000;
    // pointer to private timer
    volatile int * timer_ptr = (int *) MPCORE_PRIV_TIMER;
    // load initial value
    *timer_ptr = delay_count;
    // start timer
    *(timer_ptr + 2) = 0b001;
    // poll the timer
    while(*(timer_ptr+3) == 0){}
    // clear the interrupt register
    *(timer_ptr+3) = 1;
}

void delay_long( void ){
	// delay count of 0.01 second
    int delay_count = 2000000;
    // pointer to private timer
    volatile int * timer_ptr = (int *) MPCORE_PRIV_TIMER;
    // load initial value
    *timer_ptr = delay_count;
    // start timer
    *(timer_ptr + 2) = 0b001;
    // poll the timer
    while(*(timer_ptr+3) == 0){}
    // clear the interrupt register
    *(timer_ptr+3) = 1;
}

void delay_very_long( void ){
	// delay count of 0.1 second
    int delay_count = 600000000;
    // pointer to private timer
    volatile int * timer_ptr = (int *) MPCORE_PRIV_TIMER;
    // load initial value
    *timer_ptr = delay_count;
    // start timer
    *(timer_ptr + 2) = 0b001;
    // poll the timer
    while(*(timer_ptr+3) == 0){}
    // clear the interrupt register
    *(timer_ptr+3) = 1;
}

/*
 * Initializing and angle measures from the GYRO sensor
 */

void init_gyro( void ){
	// initialize directionality of GPIO1 (JP1) pins. CS and SPC are outputs, SDO is an input
	*jp1_gpio_dir_ptr = 0x00000007; 	// all 0 except for end which is 0111

	// enabling x, y, and z outputs and exit power down mode. -> 00001111
	write_register( CTRL_REG1, 0x0F );
}

int get_x_angular_rate( void ){
	int LSB = read_register(OUT_X_L);
	int MSB = read_register(OUT_X_H);

	return ((MSB << 8) | (LSB));
}	

int get_y_angular_rate( void ){
	int LSB = read_register(OUT_Y_L);
	int MSB = read_register(OUT_Y_H);

	return ((MSB << 8) | (LSB));
}

int get_z_angular_rate( void ){
	int LSB = read_register(OUT_Z_L);
	int MSB = read_register(OUT_Z_H);

	return ((MSB << 8) | (LSB));
}
