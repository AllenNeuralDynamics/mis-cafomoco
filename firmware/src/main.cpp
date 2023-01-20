#include <config.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <motor_controller.h>
#include <cpu_encoder.h>
#include <user_io_handler.h>
#include <core1_main.h>


// Create Object Instances.
UserIOHandler user_handler;

// Motor Controller Object GPIO setup.
// Each instance must be on its own pwm slice. See datasheet sec 1.4.3.
EnDirMotorDriver motor_drivers[NUM_BMCS]
    {{0, 1}, // PWM Slice 0
     {2, 3}, // PWM Slice 1
     {4, 5}, // PWM Slice 2
     {6, 7}, // PWM Slice 3
     {8, 9}, // PWM Slice 4
     {10, 11}};// PWM Slice 5

// Assign encoder instances to their memory location.
CPUEncoder encoders[NUM_BMCS]
    {{read_buffer_ptr, 0},
     {read_buffer_ptr, 1},
     {read_buffer_ptr, 2},
     {read_buffer_ptr, 3},
     {read_buffer_ptr, 4},
     {read_buffer_ptr, 5}};

MotorController mcs[NUM_BMCS]
    {{motor_drivers[0], encoders[0]},
     {motor_drivers[1], encoders[1]},
     {motor_drivers[2], encoders[2]},
     {motor_drivers[3], encoders[3]},
     {motor_drivers[4], encoders[4]},
     {motor_drivers[5], encoders[5]}};


/**
 * \brief true only if all mcs are idle and there's nothing in the input buffer.
 */
bool system_is_busy(void)
{
    // This could be sped up with bitfields.
    for (auto const& mc : mcs)
    {
        if (mc.is_busy())
            return true;
    }
    return user_handler.unhandled_incoming_chars();
}


void update_motor_states(void)
{
    // Update the state of all motor controllers.
    for (auto & mc : mcs)
    {
        mc.update();
    }
}


/**
 * \brief nonblocking update states of outputs.
 */
void handle_user_msg(ParsedUserMsg& user_msg)
{
    // Dispatch message.
    switch (user_msg.cmd)
    {
        case IS_BUSY:
            // Provide status indication when the system is idle.
            // Useful for polling from a serial port without cluttering output.
            if (system_is_busy())
                printf("True\r\n");
            else
                printf("False\r\n");
            break;
        case SET_SPEED:
            for (auto i = 0; i < user_msg.motor_count;++i)
            {
                uint8_t& motor_index = user_msg.motor_indexes[i];
                uint8_t& speed_in_percent = (uint8_t&)(user_msg.motor_values[i]);
                mcs[motor_index].set_speed_percentage(speed_in_percent);
            }
            break;
        case TIME_MOVE:
            // move specified motors for specified amount of time.
            for (auto i = 0; i < user_msg.motor_count;++i)
            {
                uint8_t& motor_index = user_msg.motor_indexes[i];
                MotorController::dir_t& direction = user_msg.directions[i];
                uint32_t& move_time_ms = (uint32_t&)(user_msg.motor_values[i]);
                // Do some light user input checks.
                if (mcs[motor_index].is_busy())
                {
                #ifdef DEBUG
                    printf("Skipping motor %d since it is busy.\r\n");
                #endif
                    continue;
                }
                if (mcs[motor_index].get_speed_setting() == 0)
                {
                #ifdef DEBUG
                    printf("Skipping motor %d since it has no set speed.\r\n");
                #endif
                    continue;
                }
            #ifdef DEBUG
                printf("Moving motor %d for %d\r\n",motor_index, move_time_ms);
            #endif
                mcs[motor_index].move_ms(move_time_ms, direction);
            }
            break;
        case DIST_MOVE:
            // handle the motors being issued dist move commands.
            // issue_dist_move(user_msg);
            #ifdef DEBUG
            printf("Not implemented.\r\n");
            #endif
            break;
        case HOME:
            // handle the motors being issued a home command.
            // issue_home_cmd(user_msg)
            #ifdef DEBUG
            printf("Not implemented.\r\n");
            #endif
            break;
        case HOME_IN_PLACE:
            // handle the motors being issued a home in place command.
            for (auto i = 0; i < user_msg.motor_count;++i)
            {
                uint8_t& motor_index = user_msg.motor_indexes[i];
                // Do some light user input checks.
                if (mcs[motor_index].is_busy())
                {
                #ifdef DEBUG
                    printf("Skipping motor %d since it is busy.\r\n");
                #endif
                    continue;
                }
            #ifdef DEBUG
                printf("Zeroing motor %d.\r\n",motor_index);
            #endif
                encoders[motor_index].zero();
            }
            break;
        case HOME_ALL:
            // home all motors.
            #ifdef DEBUG
            printf("Not implemented.\r\n");
            #endif
            break;
        case HOME_ALL_IN_PLACE:
            // zero out the location of all motors.
            #ifdef DEBUG
            printf("Not implemented.\r\n");
            #endif
            break;
        default:
            // shouldn't happen bc enum and message checking.
            #ifdef DEBUG
            printf("Not a valid command.\r\n");
            #endif
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

    while(true)
    {
        // Pull chars out of the USB buffer.
        user_handler.read_chars_nonblocking();
        // Check if fully-formed user input has arrived.
        // Update state of all BMCs accordingly.
        if (user_handler.new_msg())
        {
            #ifdef DEBUG
            printf("Received: %s             \r\n", user_handler.raw_buffer_); // make private.
            #endif
            user_handler.parse_msg();
            if (user_handler.msg_is_malformed())
            {
                #ifdef DEBUG
                printf("Message is malformed.\r\n");
                #endif
                user_handler.clear_msg();
                continue;
            }
            ParsedUserMsg user_msg = user_handler.get_msg();
            user_handler.clear_msg(); // Clear buffer before handling message.
            handle_user_msg(user_msg);
        }
        update_motor_states();
        #ifdef ENCODER_DEBUG
        printf("time: %07d | Enc0: %07d | Enc1: %07d | Enc2: %07d | Enc3: %07d | Enc4: %07d | Enc5: %07d\r",
                to_ms_since_boot(get_absolute_time()),
                encoders[0].get_ticks(),
                encoders[1].get_ticks(),
                encoders[2].get_ticks(),
                encoders[3].get_ticks(),
                encoders[4].get_ticks(),
                encoders[5].get_ticks());
        #endif
    }

    return 0;
}
