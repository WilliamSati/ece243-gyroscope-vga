#include "constants.h"
#include "address_map_arm.h"
#include "externalVariables.h"

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