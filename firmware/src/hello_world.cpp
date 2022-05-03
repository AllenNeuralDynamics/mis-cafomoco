#include <stdio.h>
#include "pico/stdlib.h"
#include <brushed_motor_controller.h>

// Motor Controller Objects need to be on their own slices.
BrushedMotorController bmc0(25, 1);


int main()
{
    bmc0.set_duty_cycle(25);
    stdio_usb_init();
    //while (!stdio_usb_connected()){} // wait for pc to connect to serial port.

    while(true)
    {
        //printf("Hello World!\r\n");
        bmc0.enable_output();
        sleep_ms(500);
        bmc0.disable_output();
        sleep_ms(500);
    }
    return 0;
}
