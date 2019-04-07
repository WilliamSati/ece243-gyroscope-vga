
#include "interrupt.h"
#include "board_init.h"
#include "address_map_arm.h"

void pushbutton_ISR(void)
{
volatile int * KEY_ptr = (int *)KEY_BASE;
int press;
press = *(KEY_ptr + 3); // read the pushbutton interrupt register
*(KEY_ptr + 3) = press; // Clear the interrupt


return;
}