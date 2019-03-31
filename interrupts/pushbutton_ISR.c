#include "../address_map_arm.h"
#include "../raycast-core/raycast.h"

extern volatile double player_angle; 	// normal
extern volatile int player_x_pos;
extern volatile int player_y_pos;

volatile int previous_key_state = 0;

/***************************************************************************************
 * Pushbutton - Interrupt Service Routine
****************************************************************************************/
void pushbutton_ISR(void)
{
    volatile int * KEY_ptr = (int *)KEY_BASE;
    int            press;

    press          = *(KEY_ptr + 3); // read the pushbutton interrupt register
	*(KEY_ptr + 3) = press;          // Clear the interrupt

	press = previous_key_state ^ press;
	previous_key_state = press;

	if ((press & 0x1) == 1)	//KEY0 was pressed ->RIGHT ROTATE
		player_angle = player_angle - 5 * RAY_ANGLE_INC;
	if ((press & 0x8) == 1)	//KEY3 was pressed ->LEFT ROTATE
		player_angle = player_angle + 5 * RAY_ANGLE_INC;

	int increment = 8;

	if ((press & 0x4) == 1) 	//KEY1 was pressed -> FRONT
	{
		player_y_pos = player_y_pos - increment * sind(player_angle);
		player_x_pos = player_x_pos + increment * cosd(player_angle);
	}
	
	if ((press & 0x2) == 1) 	//KEY2 was pressed -> BACK
	{
		player_y_pos = player_y_pos + increment * sind(player_angle);
		player_x_pos = player_x_pos - increment * cosd(player_angle);
	}

	return;
}