
/*
 * Implementation of functions that update position and speed of the ring as well as boat
 */ 

#include "math.h"
#include "stdbool.h" 
#include "stdlib.h"
#include "board_init.h"
#include "address_map_arm.h"

// external global variables
extern volatile int pixel_buffer_start; 
extern double dx_Boat;
extern double dy_Boat;
extern int globalRingCounter;

void updateBoatPositionAndSpeed(
	int* x_Boat, 
	int* y_Boat, 
	int width, 
	int height, 
	volatile int* keyData,
	int gyro_x_rate,
	int gyro_y_rate);

void updateRingPosition(
	int* rippleCenter_x, 
	int* rippleCenter_y, 
	int* rippleRadius, 
	volatile int* switchData,
	int furthestVisibleDistance,
	bool* lastRipple, 
	int* numRipples, 
	int* previousRippleRadius,
	double x_Boat, 
	double y_Boat);

// determines how to react to key press
void manageKeyPress(volatile int* keyData);