#include "address_map_arm.h"

extern volatile int key_dir;
extern volatile int pattern;
/*****************************************************************************
 * Interval timer interrupt service routine
 *
 * Shifts a PATTERN being displayed on the LED lights. The shift direction
 * is determined by the external variable key_dir.
 *
******************************************************************************/
void interval_timer_ISR()
{
    volatile int * interval_timer_ptr = (int *)TIMER_BASE;
    volatile int * LED_ptr            = (int *)LED_BASE; // LED address

    *(interval_timer_ptr) = 0; // Clear the interrupt

    *(LED_ptr) = pattern; // Display pattern on LED

    /* rotate the pattern shown on the LED lights */
    if (key_dir == 0) // for 0 rotate left
        if (pattern & 0x80000000)
            pattern = (pattern << 1) | 1;
        else
            pattern = pattern << 1;
    else // rotate right
        if (pattern & 0x00000001)
        pattern = (pattern >> 1) | 0x80000000;
    else
        pattern = (pattern >> 1) & 0x7FFFFFFF;

    return;
}

