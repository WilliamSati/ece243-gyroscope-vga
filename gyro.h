
/*
 * Implementation of MMA7341L accelerometer onto DE1-SOC board. The accelerometer communicates to the the DE1-SOC
 * through the A/D converter. The output values of the accelerometers lie between 0 and Vdd. These values are 
 * sent to the on chip A/D converter which can then be read by the ARM Processor.
 *
 * Datasheet: http://cache.freescale.com/files/sensors/doc/data_sheet/MMA7341L.pdf?pspll=1 
 *
 */

// Pull up voltage connected to the accelerometer
#define VDD 3.3
#define ADC_RANGE 5
// Offsets from ADC base address for each respective output
#define ADC_GYRO_X 0
#define ADC_GYRO_Y 4
#define ADC_GYRO_Z 8
// Offset from ADC base address for control register
#define ADC_CONTROL 4

// Initializing sequence for ADC
void init_gyro( void );

// Calculate angle from 12 bit value from adc
double angle_from_adc(uint8_t adc_value);