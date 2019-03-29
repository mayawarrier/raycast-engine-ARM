#include <math.h>
#include <stdbool.h>

#include "raycast.h"
#include "address_map_arm.h"
#include "Map_Data.h"

volatile int * FRAME_BUFFER_CTRL_PTR; // frame buffer controller
volatile int FRAME_BUFFER_ADDR; // the address of the frame buffer, this should be the back buffer for complex animations

void clear_screen();
void wait_for_vsync();
void draw_rectangle(int x0, int y0, int x_size, int y_size, short int rect_color);
void draw_line(int x0, int y0, int x1, int y1, short int line_color);
void plot_pixel(int x, int y, short int pixel_color);
void swap(int *x, int *y);

int main(void) 
{
	// ------------------- clear the front frame buffer -----------------

	FRAME_BUFFER_CTRL_PTR = (int *)PIXEL_BUF_CTRL_BASE;
	/* Read location of the front frame buffer from the pixel buffer controller */
	FRAME_BUFFER_ADDR = *FRAME_BUFFER_CTRL_PTR;
	// clears the front frame buffer
	clear_screen();

	// ------------------ initialize the back frame buffer -------------

	// initializes the back buffer to the start of SDRAM memory 
	*(FRAME_BUFFER_CTRL_PTR + 1) = SDRAM_BASE;

	// we draw to and clear from the back buffer now!
	FRAME_BUFFER_ADDR = *(FRAME_BUFFER_CTRL_PTR + 1); 

	// --------------------- initialize MAP_DATA -----------------------

	int i, j;
	for (i = 0; i < MAP_SIZE_X; i++) {
		for (j = 0; j < MAP_SIZE_Y; j++) {
			MAP_DATA[i][j] = 0;
		}
	}

	MAP_DATA[0][0] = 1;
	MAP_DATA[1][0] = 1;
	MAP_DATA[2][0] = 1;
	MAP_DATA[3][0] = 1;
	MAP_DATA[0][2] = 1;
	MAP_DATA[1][2] = 1;
	MAP_DATA[2][2] = 1;
	MAP_DATA[3][2] = 1;

	// draw frames
	while (true) {

		// draw frame here!
		
		// switch the front and back buffers
		wait_for_vsync();
		// update the frame buffer address
		FRAME_BUFFER_ADDR = *(FRAME_BUFFER_CTRL_PTR + 1);
	}

	return 0;
}

// clears the current frame buffer by drawing black on every pixel in the buffer
void clear_screen() {
	// increment over screen x and y
	int x, y;
	for (x = 0; x < SCREEN_SIZE_X; x++) {
		for (y = 0; y < SCREEN_SIZE_Y; y++) {
			// draw black over all pixels on the screen
			plot_pixel(x, y, 0x0000);
		}
	}
}

// waits until the front and back frame buffers are vertically synced (V-Sync)
// On most displays this should be 1/60th of a second
void wait_for_vsync() {
	// write 1 into the Buffer register, which starts the synchronization process
	// status bit will now be set to 1
	*FRAME_BUFFER_CTRL_PTR = 1;
	// get the status register, to check the status bit.
	// the status bit is 1 during vysnc and returns to 0 when sync is complete (i.e. when buffer swap is complete)
	int status_register = *(FRAME_BUFFER_CTRL_PTR + 3);
	// stay in this loop until the status bit comes back to 0
	while ((status_register & 0x01) != 0) {
		status_register = *(FRAME_BUFFER_CTRL_PTR + 3);
	}
}

// draws a rect_color rectangle at (x0, y0) from the top-left, with sizes x_size and y_size.
// draws rectangle by drawing several lines together
void draw_rectangle(int x0, int y0, int x_size, int y_size, short int rect_color) {
	// iterate over y, drawing y_size lines of x_size length
	int y;
	for (y = y0; y < y0 + y_size; y++) {
		draw_line(x0, y, x0 + x_size - 1, y, rect_color);
	}
}

// draw a line to the frame buffer using Bresenham's algorithm.
// Bresenham's algorithm increments in x, and makes decisions on whether to increment y
// based on accumulated error. If the slope is too steep, flip the coordinates to draw a smoother line
void draw_line(int x0, int y0, int x1, int y1, short int line_color) {
	
	// if the slope is too steep, we should flip the coordinates, since this draws a smoother line
	bool is_steep = abs(y1 - y0) > abs(x1 - x0);
	// flip the coordinates. later we will draw a flipped line to compensate
	if (is_steep) {
		swap(&x0, &y0);
		swap(&x1, &y1);
	}

	// if the starting coordinate is greater, swap coords since drawing the line
	// backwards is the same as drawing it forwards
	if (x0 > x1) {
		swap(&x0, &x1);
		swap(&y0, &y1);
	}

	int deltaX = x1 - x0;
	int deltaY = abs(y1 - y0);
	int accumulated_error = -(deltaX / 2);
	int y = y0;

	int y_inc;
	// set the y_increment, 1 if increasing downwards (+ve slope), -1 if increasing upwards (-ve slope)
	if (y0 < y1) {
		y_inc = 1;
	} else {
		y_inc = -1;
	}

	// incrementing x to draw the line
	int x;
	for (x = x0; x <= x1; x++) {
		// draw the line flipped since we flipped the coordinates previously
		if (is_steep) {
			plot_pixel(y, x, line_color);
		} else {
			plot_pixel(x, y, line_color);
		}
		// accumulate the error over iterations
		accumulated_error += deltaY;
		// if the error overflows, increment y so that it follows the line
		if (accumulated_error >= 0) {
			y = y + y_inc;
			accumulated_error -= deltaX;
		}
	}
}

// swaps two ints in memory
void swap(int *x, int *y) {
	int temp = *x;
	*x = *y;
	*y = temp;
}

// plot a pixel at x, y by writing to the frame buffer
void plot_pixel(int x, int y, short int pixel_color) 
{
	*(short int *)(FRAME_BUFFER_ADDR + (y << 10) + (x << 1)) = pixel_color;
}