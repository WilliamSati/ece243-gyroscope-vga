
/*
 * Implementation of SPI communication between L3GD20 gyroscope and ARM.
 */

#include "gyro.h"
#include "address_map_arm.h"

volatile uint32_t* jp1_gpio_data_ptr = JP1_BASE;
volatile uint32_t* jp1_gpio_dir_ptr = JP1_BASE + 4;

/*
 * Implementation of SPI for the L3GD20. Simple read and write to registers
 */

// function that reads register from a specified address on the L3GD20. Protocol specified on data sheet
uint8_t read_register( uint8_t address ){
	uint8_t register_val = 0x00;

	// drive clock high before CS is driven low
	clock_high();

	// drive CS pin low to start transmission
	*jp1_gpio_data_ptr &= ~(1 << CS);

	// drive RW to 1 for register read
	SDI_high();
	clock_low();
	clock_high();

	// drive MS to 0, no auto-incremented reading
	SDI_low();
	clock_low();
	clock_high();

	// send address bits, 6 bits
	for(uint8_t i=0; i<6; i++){
		if((address & (1 << i)) != 0){
			SDI_high();
		}
		else {
			SDI_low();
		}
		clock_low();
		clock_high();
	}

	// recieve MSB to LSB through the SDO
	for(uint8_t i=7; i>=0; i--){
		clock_low();
		if((*jp1_gpio_data_ptr & (1 << SDO)) != 0){
			register_val |= (1 << i);
		}
		else {
			register_val &= ~(1 << i);
		}
		clock_high();
	}

	// finish transmission, drive CS high
	*jp1_gpio_data_ptr |= (1 << CS);
}

// function that writes to a register on the L3GD20
void write_register( uint8_t address, uint8_t data ){

	// drive clock high before CS is driven low
	clock_high();

	// drive CS pin low to start transmission
	*jp1_gpio_data_ptr &= ~(1 << CS);

	// drive RW to 0 for register write
	SDI_low();
	clock_low();
	clock_high();

	// drive MS to 0, no auto-incremented reading
	SDI_low();
	clock_low();
	clock_high();

	// send address bits, 6 bits
	for(uint8_t i=0; i<6; i++){
		// extract bits from address
		if((address & (1 << i)) != 0){
			SDI_high();
		}
		else {
			SDI_low();
		}
		clock_low();
		clock_high();
	}

	// recieve MSB to LSB through the SDO
	for(uint8_t i=7; i>=0; i--){
		// extract bits from data
		if((data & (1 << i)) != 0){
			SDI_high();
		}
		else {
			SDI_low();
		}
		clock_low();
		clock_high();
	}

	// finish transmission, drive CS high
	*jp1_gpio_data_ptr |= (1 << CS);
}
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


/*
 * Initializing and angle measures from the GYRO sensor
 */

void init_gyro( void ){
	// initialize directionality of GPIO1 (JP1) pins. CS and SPC are outputs, SDO is an input
	*jp1_gpio_dir_ptr = 0x00000007; 	// all 0 except for end which is 0111

	// enabling x, y, and z outputs and exit power down mode. -> 00001111
	write_register( CTRL_REG1, 0x0F );
}

uint32_t get_x_angular_rate( void ){
	uint32_t LSB = read_register(OUT_X_L);
	uint32_t MSB = read_register(OUT_X_H);

	return ((MSB << 8) || (LSB));
}	

uint32_t get_y_angular_rate( void ){
	uint32_t LSB = read_register(OUT_Y_L);
	uint32_t MSB = read_register(OUT_Y_H);

	return ((MSB << 8) || (LSB));
}

uint32_t get_z_angular_rate( void ){
	uint32_t LSB = read_register(OUT_Z_L);
	uint32_t MSB = read_register(OUT_Z_H);

	return ((MSB << 8) || (LSB));
}