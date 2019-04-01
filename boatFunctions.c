#include "constants.h"
#include "address_map_arm.h"
#include "externalVariables.h"
#include "manageUserInputFunctions.h"
#include "math.h"

void updateBoatPositionAndSpeed(int* x_Boat, int* y_Boat, int width, int height, volatile int* keyData){
	
	//poll the user's input to updsate dx_Boat and dy_Boat
		if(*keyData!=0){
			manageKeyPress(keyData);
		}
	
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