#include <config.h>
#include <pico/stdlib.h>
#include <pico/multicore.h>
#include <motor_controller.h>
#include <cpu_encoder.h>
#include <user_io_handler.h>
#include <core1_main.h>


// Create Object Instances.
UserIOHandler user_handler;

// Motor Controller Objects need to be on their own slices.
EnDirMotorDriver motor_drivers[NUM_BMCS]
    {EnDirMotorDriver(0, 1), // PWM Slice 0
     EnDirMotorDriver(2, 3), // PWM Slice 1
     EnDirMotorDriver(4, 5), // PWM Slice 2
     EnDirMotorDriver(6, 7)};// PWM Slice 3

// Assign encoder instances to their memory location.
CPUEncoder encoders[NUM_BMCS]
    {CPUEncoder(read_buffer_ptr, 0),
     CPUEncoder(read_buffer_ptr, 1),
     CPUEncoder(read_buffer_ptr, 2),
     CPUEncoder(read_buffer_ptr, 3)};

MotorController mcs[NUM_BMCS]
    {MotorController(motor_drivers[0], encoders[0]),
     MotorController(motor_drivers[1], encoders[1]),
     MotorController(motor_drivers[2], encoders[2]),
     MotorController(motor_drivers[3], encoders[3])};


/**
 * \brief true if all mcs are idle. false otherwise.
 */
bool system_is_busy(void)
{
    // This could be sped up with bitfields.
    for (auto const& mc : mcs)
    {
        if (mc.is_busy())
            return true;
    }
    return false;
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
            if (!system_is_busy())
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
            handle_user_msg(user_msg);
            user_handler.clear_msg();
        }
        update_motor_states();
        #ifdef DEBUG
        printf("time: %07d | Enc0 ticks: %07d\r",
                to_ms_since_boot(get_absolute_time()), encoders[0].get_ticks());
        #endif
    }

    return 0;
}
