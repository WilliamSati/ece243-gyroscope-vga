
#include "graphics_animation.h"

void updateBoatPositionAndSpeed(
	int* x_Boat, int* y_Boat, 
	int width, int height, 
	volatile int* keyData,
	int gyro_x_rate,
	int gyro_y_rate){
	
	//poll the user's input to updsate dx_Boat and dy_Boat
		if(*keyData!=0){
			manageKeyPress(keyData);
		}
	
	//convert gyroscope output from twos compliment to integer value
		int gyro_x_dec;
		if(gyro_x_rate && (1 << 15)){
			gyro_x_dec = ((~gyro_x_rate) + 1)*(-1);
		}
		else{
			gyro_x_dec = gyro_x_rate;
		}

		int gyro_y_dec;
		if(gyro_y_rate && (1 << 15)){
			gyro_y_dec = ((~gyro_y_rate) + 1)*(-1);
		}
		else{
			gyro_y_dec = gyro_y_rate;
		}

	// adjust the value of dx_boat and dy_boat
		dx_Boat += gyro_x_dec/100;
		dy_Boat += gyro_y_dec/100;

	//make sure the boat's position will be inbounds if we add dx_Boat and dy_Boat. If it's not, kill the boat's speed.
		if(*x_Boat + (int)dx_Boat > 320-(width-1)){
			dx_Boat = 0;
			*x_Boat = 320-(width-1);
		}
		else if(*x_Boat + (int)dx_Boat < 0){
			dx_Boat = 0;
			*x_Boat = 0;
		}
		else{
			*x_Boat = *x_Boat + dx_Boat;
		}
		
		if(*y_Boat + dy_Boat > 240-(height-1)){
			dy_Boat = 0;
			*y_Boat = 240-(height-1);
		} else if(*y_Boat + dy_Boat < 0){
			dy_Boat = 0;
			*y_Boat = 0;
		} else{
			*y_Boat = *y_Boat + dy_Boat;
		}
		
		//account for the drag force of the water which decelerates the boat.
		if(dx_Boat>0){
			dx_Boat -= pow(dx_Boat,2)/75+0.01;
		}
		if(dx_Boat<0){
			dx_Boat += pow(dx_Boat,2)/75+0.01;
		}
		if(dy_Boat>0){
			dy_Boat -= pow(dy_Boat,2)/75+0.01;
		}
		if(dy_Boat<0){
			dy_Boat += pow(dy_Boat,2)/75+0.01;
		}
}

void updateRingPosition(
	int* rippleCenter_x, 
	int* rippleCenter_y, 
	int* rippleRadius, 
	volatile int* switchData,
	int furthestVisibleDistance, 
	bool* lastRipple, 
	int* numRipples, 
	int* previousRippleRadius, 
	double x_Boat, double y_Boat){
	int i = 0;
	
	//if the switch is on, create another ring
	if(globalRingCounter==10){
		globalRingCounter=0;
		*numRipples+=1;

		rippleCenter_x[*numRipples-1] = x_Boat;
		rippleCenter_y[*numRipples-1] = y_Boat;
		rippleRadius[*numRipples-1] = 0;
		lastRipple[*numRipples-1] = 0;
		previousRippleRadius[*numRipples-1] = rippleRadius[*numRipples-1];
	}
	
	
	//for all of the ripples, update their location
	for(; i < *numRipples ; i++){
		if(rippleRadius[i] > furthestVisibleDistance){
			lastRipple[i] = 1;	//this will lead to a ripple being deleted.	
		}else {
			rippleRadius[i]+=1;
			lastRipple[i] = 0;
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
		dx_Boat+=0.25;
	}
	if(arrayOfPressedKeys[1]){
		dy_Boat-=0.25;
	}
	if(arrayOfPressedKeys[2]){
		dy_Boat+=0.25;
	}
	if(arrayOfPressedKeys[3]){
		dx_Boat-=0.25;
	}	
}