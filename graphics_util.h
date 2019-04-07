// Functions use plot_pixel to draw things to the back buffer. 
// Use wait_for_vsync to swap the buffer address you are writting to.

#include "stdbool.h" 
#include "board_init.h"
#include "address_map_arm.h"
#include "graphics_animation.h"

void draw_line(int x0, int x1, int y0, int y1, short int color);
void plot_pixel(int x, int y, short int line_color);
void clear_screen();
void drawBox(int xLeft, int yTop, int width, int height, short int color_Xs);
void wait_for_vsync(volatile int* pixelStatusPtr, volatile int* pixel_ctrl_ptr);
void drawRing(int rippleRadius, int rippleCenter_x,int rippleCenter_y,short int ring_Color);
bool screenContains(int x, int y);
void plotEightSymmetricPixels(int xCenter, int yCenter, int x, int y, short int ring_Color);
