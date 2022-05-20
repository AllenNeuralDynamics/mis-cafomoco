#include <config.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <brushed_motor_controller.h>
#include <user_io_handler.h>
#include <core1_main.h>


// Motor Controller Objects need to be on their own slices.
UserIOHandler user_handler;

BrushedMotorController bmcs[NUM_BMCS]
    {BrushedMotorController(0, 1), // Slice 0
     BrushedMotorController(2, 3), // Slice 1
     BrushedMotorController(4, 5), // Slice 2
     BrushedMotorController(6, 7)};// Slice 3



/**
 * \brief true if all bmcs are idle. false otherwise.
 */
bool system_is_busy(void)
{
    // This could be sped up with bitfields.
    for (auto const& bmc : bmcs)
    {
        if (bmc.state_ != BrushedMotorController::IDLE)
            return true;
    }
    return false;
}


void update_motor_states(void)
{
    // Update the state of all motor controllers.
    for (auto & bmc : bmcs)
    {
        bmc.update();
    }
}


/**
 * \brief nonblocking update states of outputs.
 */
void handle_user_msg(ParsedUserMsg& user_msg)
{
    switch (user_msg.cmd)
    {
        case IS_BUSY:
            // Tell the user if any BMC is moving.
            if (system_is_busy())
                printf("True\r\n");
            else
                printf("False\r\n");
            break;
        case SET_SPEED:
            for (auto i = 0; i < user_msg.motor_count;++i)
            {
                uint8_t& motor_index = user_msg.motor_indexes[i];
                uint8_t& duty_cycle_percent = user_msg.motor_values[i];
                bmcs[motor_index].set_duty_cycle(duty_cycle_percent);
            }
            break;
        case TIME_MOVE:
            // move specified motors for specified amount of time.
            // issue_time_move(user_msg);
            for (auto i = 0; i < user_msg.motor_count;++i)
            {
                uint8_t& motor_index = user_msg.motor_indexes[i];
                uint8_t& move_time_ms = user_msg.motor_values[i];
                bmcs[motor_index].move_ms(move_time_ms);
            }
            break;
        case DIST_MOVE:
            // handle the motors being issued dist move commands.
            // issue_dist_move(user_msg);
            printf("Not implemented.\r\n");
            break;
        case HOME:
            // handle the motors being issued a home command.
            // issue_home_cmd(user_msg)
            printf("Not implemented.\r\n");
            break;
        case HOME_IN_PLACE:
            // handle the motors being issued a home in place command.
            printf("Not implemented.\r\n");
            break;
        case HOME_ALL:
            // home all motors.
            printf("Not implemented.\r\n");
            break;
        case HOME_ALL_IN_PLACE:
            // zero out the location of all motors.
            printf("Not implemented.\r\n");
            break;
        default:
            // shouldn't happen bc enum and message checking.
            printf("Not a valid command.\r\n");
            break;
    }
}


// Core0 main.
int main()
{
    // init usb serial connection. Blocks until connected.
    user_handler.init();
    init_encoder_pins();

    // launch the encoder reading/updating process on Core1.
    multicore_launch_core1(core1_main);


    uint32_t* read_buffer_ptr_cpy;
    while(true)
    {
        sleep_ms(500);
        // Note: copy the read ptr so we can finish reading it out.
        read_buffer_ptr_cpy = read_buffer_ptr;
        printf("Encoder data: %d | %d | %d | %d\r\n", read_buffer_ptr_cpy[0],
                                                      read_buffer_ptr_cpy[1],
                                                      read_buffer_ptr_cpy[2],
                                                      read_buffer_ptr_cpy[3]);
    }
    // rest is unreachable for now.

    while(true)
    {
        // Pull chars out of the USB buffer.
        user_handler.read_chars_nonblocking();
        // Check if fully-formed user input has arrived.
        // Update state of all BMCs accordingly.
        if (user_handler.new_msg())
        {
            printf("Received: %s\r\n", user_handler.raw_buffer_); // make private.
            user_handler.parse_msg();
            if (user_handler.msg_is_malformed())
            {
                printf("Message is malformed.\r\n");
                user_handler.clear_msg();
                continue;
            }
            ParsedUserMsg user_msg = user_handler.get_msg();
//            printf("CMD: %d\r\n", user_msg.cmd);
//            printf("Motor count: %d\r\n", user_msg.motor_count);
//            printf("Motors: ");
//            for (auto i=0;i<user_msg.motor_count;++i)
//                printf("%d, ",user_msg.motor_indexes[i]);
//            printf("\r\n");
            handle_user_msg(user_msg);
            user_handler.clear_msg();
        }

        update_motor_states();
    }

    return 0;
}
