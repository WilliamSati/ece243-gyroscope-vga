/*
 * Implementation of gyroscope functionality using the L3GD20 Gryo
 * Data sheet: https://www.pololu.com/file/0J563/L3GD20.pdf
 *
 * Communication between ARM and L3GD20 will be done through SPI. Thus, CS pin should be driven low at the start of
 * transmission.
 * 
 * Note: ONLY read from addresses specified in this file. Accidentally reading an inappropriate 
 * address can cause permenant damage to the gyrocope
 */

#include "address_map_arm.h"

// address mappings to control registers
#define 	WHO_AM_I 	0X0F    // ID : 11010100
#define 	CTRL_REG1 	0x20 	// default contents: 00000111
#define 	CTRL_REG2 	0x21	// default contents: 00000000
#define 	CTRL_REG3 	0x22	// default contents: 00000000
#define 	CTRL_REG4 	0x23	// default contents: 00000000
#define 	CTRL_REG5 	0x24	// default contents: 00000000
#define 	REFERENCE 	0x25    // default contents: 00000000

// address mappings to output registers
#define 	OUT_TEMP 	0x26
#define 	STATUS_REG 	0x27 	
#define 	OUT_X_L 	0x28
#define 	OUT_X_H 	0x29
#define 	OUT_Y_L 	0x2A
#define 	OUT_Y_H 	0X2B
#define 	OUT_Z_L		0x2C
#define 	OUT_Z_H 	0x2D

// pin connections between gyro and ARM
#define 	IO_BASE 	JP1_BASE
#define 	CS 			0 		// chip select
#define 	SPC 		1	 	// Serial Port Clock
#define 	SDI 		2		// Serial Data In
#define 	SDO 		3		// Serial Data Out


// Utility functions to read registers from gyro via SPI. All registers on L3GD20 chip are one byte. 
int read_register( int address );
void write_register( int address, int data );

// Covinience functions for writing toggling inputs to gyroscope
void clock_low( void );
void clock_high( void );
void SDI_low( void );
void SDI_high( void ); 

// Delay functions for debugging
void delay(void);
void delay_long(void);
void delay_very_long(void);

// Initializing and reading data from gyro
void init_gyro( void );

// Getting useful angular measurements
int get_x_angular_rate( void );
int get_y_angular_rate( void );
int get_z_angular_rate( void );

