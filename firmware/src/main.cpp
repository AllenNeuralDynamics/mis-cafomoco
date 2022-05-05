//#include <stdio.h>
#include <pico/stdlib.h>
#include <brushed_motor_controller.h>
#include <user_io_handler.h>

// Constants
#define NUM_BMCS (4)


// Motor Controller Objects need to be on their own slices.
UserIOHandler user_handler;

BrushedMotorController bmcs[NUM_BMCS]
    {BrushedMotorController(0, 1), // Slice 0
     BrushedMotorController(2, 3), // Slice 1
     BrushedMotorController(4, 5), // Slice 2
     BrushedMotorController(6, 7)};// Slice 3


int main()
{
    user_handler.init(); // init usb serial connection. Blocks until connected.

    while(true)
    {
        user_handler.read_chars_nonblocking();
        // Check if fully-formed user input has arrived.
        // i.e: Check if the last value in the serial buffer is '\r'
        // Update state of all BMCs accordingly.
        if (user_handler.new_msg())
        {
            if (user_handler.msg_is_malformed())
            {
                user_handler.clear_msg();
                continue;
            }
            ParsedUserMsg user_msg = user_handler.get_msg();

            switch (user_msg.cmd)
            {
                case IS_BUSY:
                    // Tell the user if any BMC is moving.
                    // printf(something);
                    break;
                case TIME_MOVE:
                    // move specified motors for specified amount of time.
                    // issue_time_move(user_msg);
                    break;
                case DIST_MOVE:
                    // handle the motors being issued dist move commands.
                    // issue_dist_move(user_msg);
                    break;
                case HOME:
                    // handle the motors being issued a home command.
                    // issue_home_cmd(user_msg)
                    break;
                case HOME_IN_PLACE:
                    // handle the motors being issued a home in place command.
                    break;
                case HOME_ALL:
                    // home all motors.
                    break;
                case HOME_ALL_IN_PLACE:
                    // zero out the location of all motors.
                    break;
                default:
                    // shouldn't happen bc enum and message checking.
                    break;
            }
        }

        // Update the state of all motor controllers.
        for (auto const& bmc : bmcs){}
    }

    return 0;
}
