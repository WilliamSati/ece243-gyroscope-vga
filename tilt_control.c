
/*
 * Implementation of MMA7341L accelerometer onto DE1-SOC board. The accelerometer communicates to the the DE1-SOC
 * through the A/D converter. The output values of the accelerometers lie between 0 and Vdd. These values are 
 * sent to the on chip A/D converter which can then be read by the ARM Processor.
 *
 * Datasheet: http://cache.freescale.com/files/sensors/doc/data_sheet/MMA7341L.pdf?pspll=1 
 *
 * The accelerometer output will be connected to Channel 0 ADC
 */

#include "address_map_arm.h"
#include "gyro.h"

// declare pointers to the adc
volatile uint32_t* adc_auto_update = ADC_BASE + ADC_CONTROL;
volatile uint32_t* gyro_x = ADC_GYRO_X;
volatile uint32_t* gyro_y = ADC_GYRO_Y;
volatile uint32_t* gyro_z = ADC_GYRO_Z;

void init_gyro( void ){
	// enable auto update for the ADC
	*adc_auto_update = 1;
}

// calculates the tilt angle from the readings from the adc. Voltage must be converted to
double angle_from_adc( uint32_t adc_value ){
	double voltage = (adc_value*ADC_RANGE)/(1 << 12);
	// compute the arcsin
	
}

double arcsin( double ratio ){
	// fifth order taylot series approximation
	double angle = ratio + 0.5*ratio*ratio*ratio + 0.075*ratio*ratio*ratio*ratio*ratio; 
}