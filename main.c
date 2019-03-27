#include <math.h>
#include <stdbool.h>

volatile int * FRAME_BUFFER_CTRL_PTR; // frame buffer controller
volatile int FRAME_BUFFER_ADDR; // the address of the frame buffer, this should be the back buffer for complex animations

#define SCREEN_SIZE_X 320
#define SCREEN_SIZE_Y 240
#define NUM_RECTANGLES 8
#define NUM_COLORS 7
#define RECT_SIZE_X 12
#define RECT_SIZE_Y 8

void clear_screen();
void wait_for_vsync();
void draw_rectangle(int x0, int y0, int x_size, int y_size, short int rect_color);
void draw_line(int x0, int y0, int x1, int y1, short int line_color);
void plot_pixel(int x, int y, short int pixel_color);
void swap(int *x, int *y);

int main(void) 
{
	// ------------------- clear the front frame buffer -----------------

	FRAME_BUFFER_CTRL_PTR = (int *)0xFF203020;
	/* Read location of the front frame buffer from the pixel buffer controller */
	FRAME_BUFFER_ADDR = *FRAME_BUFFER_CTRL_PTR;
	// clears the front frame buffer
	clear_screen();

	// ------------------ initialize the back frame buffer -------------

	// initializes the back buffer to the start of SDRAM memory 
	*(FRAME_BUFFER_CTRL_PTR + 1) = 0xC0000000;

	// we draw to and clear from the back buffer now!
	FRAME_BUFFER_ADDR = *(FRAME_BUFFER_CTRL_PTR + 1); 

	/* ------- Animate 8 bouncing rectangles on the screen, connected by lines ---- */

	// -------------- generate random initial velocities, locations, and colors --------

	short int RECT_COLOR_PALETTE[NUM_COLORS];

	int RECT_X_POS[NUM_RECTANGLES];
	int RECT_Y_POS[NUM_RECTANGLES];
	int RECT_X_VEL[NUM_RECTANGLES];
	int RECT_Y_VEL[NUM_RECTANGLES];
	short int RECT_COLORS[NUM_RECTANGLES];

	// fill in all possible rectangle colors
	RECT_COLOR_PALETTE[0] = 0xF800; // red
	RECT_COLOR_PALETTE[1] = 0x07E0; // green
	RECT_COLOR_PALETTE[2] = 0x001F; // blue
	RECT_COLOR_PALETTE[3] = 0xFFE0; // yellow
	RECT_COLOR_PALETTE[4] = 0xF81F; // magenta
	RECT_COLOR_PALETTE[5] = 0x07FF; // cyan
	RECT_COLOR_PALETTE[6] = 0xFCA0; // orange

	int i;
	for (i = 0; i < NUM_RECTANGLES; i++) {
		RECT_X_POS[i] = rand() % (SCREEN_SIZE_X - RECT_SIZE_X);
		RECT_Y_POS[i] = rand() % (SCREEN_SIZE_Y - RECT_SIZE_Y);
		RECT_X_VEL[i] = rand() % 5 - 2;
		RECT_Y_VEL[i] = rand() % 5 - 2;
		RECT_COLORS[i] = RECT_COLOR_PALETTE[rand() % NUM_COLORS];
	}

	// do animation
	while (true) {

		// clear previous drawings
		clear_screen();

		// update positions based on velocities
		int i;
		for (i = 0; i < NUM_RECTANGLES; i++) {
			RECT_X_POS[i] += RECT_X_VEL[i];
			RECT_Y_POS[i] += RECT_Y_VEL[i];
		}

		// bounds checking, to bounce rectangles off the screeen edges
		for (i = 0; i < NUM_RECTANGLES; i++) {
			if (RECT_X_POS[i] < 0) {
				RECT_X_POS[i] = 0;
				RECT_X_VEL[i] = -RECT_X_VEL[i];
			}
			if (RECT_X_POS[i] + RECT_SIZE_X > SCREEN_SIZE_X) {
				RECT_X_POS[i] = SCREEN_SIZE_X - RECT_SIZE_X;
				RECT_X_VEL[i] = -RECT_X_VEL[i];
			}
			if (RECT_Y_POS[i] < 0) {
				RECT_Y_POS[i] = 0;
				RECT_Y_VEL[i] = -RECT_Y_VEL[i];
			}
			if (RECT_Y_POS[i] + RECT_SIZE_Y > SCREEN_SIZE_Y) {
				RECT_Y_POS[i] = SCREEN_SIZE_Y - RECT_SIZE_Y;
				RECT_Y_VEL[i] = -RECT_Y_VEL[i];
			}
		}

		// --------------------------- draw rectangles and lines -------------------------

		// pre-compute for speed
		int HALF_RECT_SIZE_X = RECT_SIZE_X / 2;
		int HALF_RECT_SIZE_Y = RECT_SIZE_Y / 2;

		// updated as we draw all the lines
		int line_start_x, line_start_y, line_end_x, line_end_y;

		// redraw rectangles and lines
		for (i = 0; i < NUM_RECTANGLES; i++) {

			draw_rectangle(RECT_X_POS[i], RECT_Y_POS[i], RECT_SIZE_X, RECT_SIZE_Y, RECT_COLORS[i]);

			// draw line from this rectangle center to the next rectangle center
			line_start_x = RECT_X_POS[i] + HALF_RECT_SIZE_X;
			line_start_y = RECT_Y_POS[i] + HALF_RECT_SIZE_Y;

			if (i < NUM_RECTANGLES - 1) {
				// not the last rectangle, connect this rectangle to the next rectangle
				line_end_x = RECT_X_POS[i + 1] + HALF_RECT_SIZE_X;
				line_end_y = RECT_Y_POS[i + 1] + HALF_RECT_SIZE_Y;
				draw_line(line_start_x, line_start_y, line_end_x, line_end_y, 0xFFFF);
			}
			else {
				// the last rectangle must connect to the first rectangle
				line_end_x = RECT_X_POS[0] + HALF_RECT_SIZE_X;
				line_end_y = RECT_Y_POS[0] + HALF_RECT_SIZE_Y;
				draw_line(line_start_x, line_start_y, line_end_x, line_end_y, 0xFFFF);
			}
		}

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