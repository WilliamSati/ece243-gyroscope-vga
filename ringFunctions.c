#include "constants.h"
#include "address_map_arm.h"
#include "externalVariables.h"
#include "stdbool.h" 
#include "stdlib.h"

void updateRingPosition(int* rippleCenter_x, int* rippleCenter_y, int* rippleRadius, volatile int* switchData,int furthestVisibleDistance, bool* lastRipple, int* numRipples, int* previousRippleRadius){
	int i = 0;
	
	//if the switch is on, create another ring
	
	if((*switchData & 1) == 0){
		;
	}else{
		*numRipples+=1;
		rippleCenter_x[*numRipples-1] = rand()%320;
		rippleCenter_y[*numRipples-1] = rand()%240;
		rippleRadius[*numRipples-1] = 0;
		lastRipple[*numRipples-1] = 0;
		previousRippleRadius[*numRipples-1] = rippleRadius[*numRipples-1];
	}
	
	
	//for all of the ripples, update their location
for(; i < *numRipples ; i++){
	
	if(rippleRadius[i] > furthestVisibleDistance){
		lastRipple[i] = 1;	//this will lead to a ripple being deleted.	
	}else{
		rippleRadius[i]+=1;
		lastRipple[i] = 0;
	}
}
	
}