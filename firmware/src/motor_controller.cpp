#include <motor_controller.h>

#ifdef DEBUG
constexpr const char* const MotorController::state_as_str[];
#endif

MotorController::MotorController(EnDirMotorDriver& motor_driver,
                                 CPUEncoder& encoder)
    :motor_driver_{motor_driver}, encoder_{encoder},
    stuck_moving_{false}, state_{MotorController::BMCState::IDLE},
    set_speed_{0}, move_direction_{MotorController::FORWARD}
{
    motor_driver_.set_duty_cycle(0);
}


MotorController::~MotorController()
{
    motor_driver_.disable_output();
}


void MotorController::set_speed_percentage(uint8_t speed_percentage)
{
    // Clear output duty cycle on startup.
    motor_driver_.set_duty_cycle(speed_percentage);

    // Save speed setting.
    set_speed_ = speed_percentage;
}


void MotorController::set_pwm_frequency(uint32_t freq_hz)
{
    motor_driver_.set_pwm_frequency(freq_hz);
}


void MotorController::move_ms(uint32_t milliseconds,
                              MotorController::dir_t direction)
{
    // Bail early and ignore user input if we can't accept it.
    if (is_busy())
        return;

    // increase the movement time.
    set_move_time_ms_ = milliseconds;
    move_direction_ = direction;
}


void MotorController::move_relative_angle(float angle)
{
    // Bail early and ignore user input if we can't accept it.
    if (is_busy())
        return;

    // increase the movement angle.
    //TODO;
}


void MotorController::update()
{
    // Cleanup inputs:
    // Nothing to do!

    // Handle state transition logic (state + inputs that cause a state change).
    // (Mealy-style State machine.)
    BMCState next_state{state_};
    switch (state_)
    {
        case IDLE:
        {
            // Check inputs.
            if (set_move_time_ms_ > 0 && set_speed_ > 0) // Only move if a speed was set.
            {
                next_state = TIME_MOVE;
            }
            else if (set_move_angle_ticks_ != 0 &&
                     set_speed_ > 0) // Only move if a speed was set.
            {
                next_state = DIST_MOVE;
            }
            else {}
            break;
        }
        case TIME_MOVE:
        {
            uint32_t curr_time_ms = to_ms_since_boot(get_absolute_time());
            uint32_t end_time_ms = move_start_time_ms_ + set_move_time_ms_;
            // Check if we're stuck moving.
            if (curr_time_ms < end_time_ms && stuck_moving_)
                next_state = ERROR;
            // Check if we're done moving.
            else if (curr_time_ms >= end_time_ms)
                next_state = IDLE;
            break;
        }
        case DIST_MOVE:
        {
            // Check elapsed distance within error bars.
            break;
        }
        case HOMING:
        {
            // Check the state of the GPIOs. Halt motor if triggered.
            break;
        }
        default:
            break;
    }

    // Handle Ouputs on State Transition
    if (state_ == IDLE && next_state == TIME_MOVE)
    {
        move_start_time_ms_ = to_ms_since_boot(get_absolute_time());
        motor_driver_.set_dir(bool(move_direction_));
        motor_driver_.enable_output();
    }
    else if (state_ == TIME_MOVE && next_state == IDLE)
    {
        set_move_time_ms_ = 0;
        move_start_time_ms_ = 0;
        motor_driver_.disable_output();
    }
    else {}

#ifdef DEBUG
    if (state_ != next_state)
        printf("Changing states from: %s to %s\r\n", state_as_str[state_],
                                                     state_as_str[next_state]);
#endif

    // Change states.
    state_ = next_state;
}
