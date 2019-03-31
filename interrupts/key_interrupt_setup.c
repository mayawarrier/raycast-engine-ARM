#include "../address_map_arm.h"

void set_A9_IRQ_stack(void);
void config_GIC(void);
void config_KEYs(void);
void enable_A9_interrupts(void);

void config_key_interrupts(void)
{
	// configure KEY interrupts

    set_A9_IRQ_stack();      // initialize the stack pointer for IRQ mode
    config_GIC();            // configure the general interrupt controller
    config_KEYs();           // configure pushbutton KEYs to generate interrupts

    enable_A9_interrupts(); // enable interrupts
}

/* setup the KEY interrupts in the FPGA */
void config_KEYs()
{
    volatile int * KEY_ptr = (int *)KEY_BASE; // pushbutton KEY address
    *(KEY_ptr + 2) = 0xF; //enable interrupts for all KEYS	
}