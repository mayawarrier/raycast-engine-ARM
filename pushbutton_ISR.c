#include "address_map_arm.h"

extern volatile int key_dir;
extern volatile int pattern;
volatile double player_angle = 90.0; 	// normal
extern volatile int player_x_pos;
extern volatile int player_y_pos;
volatile int increment_val = 8; 
/***************************************************************************************
 * Pushbutton - Interrupt Service Routine
 *
 * This routine toggles the key_dir variable from 0 <-> 1
****************************************************************************************/
void pushbutton_ISR(void)
{
    volatile int * KEY_ptr = (int *)KEY_BASE;
    int            press;

    press          = *(KEY_ptr + 3); // read the pushbutton interrupt register
    *(KEY_ptr + 3) = press;          // Clear the interrupt

    //key_dir ^= 1; // Toggle key_dir value, using XOR.okay this reverses the direction of the lights flashing. 
     if (*(KEY_ptr) == 0x0)	//KEY0 was pressed ->RIGHT ROTATE
	player_angle = player_angle - 5*RAY_ANGLE_INC;     	
     if(*(KEY_ptr == 0x3))	//KEY3 was pressed ->LEFT ROTATE
	player_angle = player_angle + 5*RAY_ANGLE_INC;
     if(*(KEY_ptr == 0x1)) 	//KEY1 was pressed -> FRONT
	{
		if(player_angle == 90)
			player_y_pos = player_y_pos - increment_val;
		else
		   {
                   	player_y_pos = player_y_pos - increment*cosd(player_angle);
			player_x_pos = player_x_pos - increment*sind(player_angle); 
		   }
	}
	if(*(KEY_ptr == 0x2))	//KEY2 was pressed -> BACK
	{

		if(player_angle == 90)
			player_y_pos = player_y_pos + increment_val;
		else
		   {
                   	player_y_pos = player_y_pos + increment_val*cosd(player_angle);
			player_x_pos = player_x_pos + increment_val*sind(player_angle); 
		   }	
	}
		
    return;
}
