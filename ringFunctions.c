#include "constants.h"
#include "address_map_arm.h"
#include "externalVariables.h"
#include "stdbool.h" 
#include "stdlib.h"
void updateRingPosition(int* rippleCenter_x, int* rippleCenter_y, int* rippleRadius, volatile int* switchData, bool* drawingRipple, int* furthestPoint,int* furthestVisibleDistance,bool* lastRipple, int* numRipples){
	int i = 0;
	int originalNumberOfRipples = *numRipples;
for(; i< originalNumberOfRipples ; i++){
	if(!drawingRipple[i]){
		if((*switchData & 1) == 0){ //check switch 1. If it's on, make a ripple, if it's not, don't make a ripple.
			;
		}else{
			drawingRipple[i] = 1;
			rippleCenter_x[i] = rand()%320;
			rippleCenter_y[i] = rand()%240;
			rippleRadius[i] = 0;
			*numRipples +=1 ;
			*furthestVisibleDistance = 400;
		}
		return;		
	}
	
	if(rippleRadius[i] > *furthestVisibleDistance){
		drawingRipple[i] = 0;
		lastRipple[i] = 1;
		
		
	}else{
		rippleRadius[i]+=1;
	}
}
	
}