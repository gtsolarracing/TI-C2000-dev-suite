#include "F2806x_Device.h"
#include "F2806x_Examples.h"
#include "clocks.h"
#include "gpio.h"

//Basic MCU "hello world" blinking LED project
//Arduino style function definitions
void start();
void loop();

void main(void) {
	EALLOW;
	SysClkInit(NINETY);
	start();
    EDIS;
    while (1) {
    	serviceWatchog();
    	loop();
    }
}

void start(){
	GpioOutputInit(1);
}

void loop(){
	GpioTogglePin(1);
	DELAY_US(1e6); // Wait 1s until toggle again.
}
