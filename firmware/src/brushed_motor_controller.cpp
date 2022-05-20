#include <brushed_motor_controller.h>
// Assumes a 125MHz system clock.

BrushedMotorController::BrushedMotorController(uint8_t torque_pwm_pin,
                                               uint8_t dir_pin)
    :torque_pwm_pin_{torque_pwm_pin}, dir_pin_{dir_pin}, stuck_moving_{false},
     state_{BrushedMotorController::BMCState::IDLE}
{
// inpired by:
// https://github.com/raspberrypi/pico-examples/blob/master/pwm/hello_pwm/hello_pwm.c#L14-L29
    // Allocate torque_pwm_pin for pwm; allocate dir pin as output.
    gpio_set_function(torque_pwm_pin_, GPIO_FUNC_PWM);

    // Initialize direction pin and default to output 0.
    gpio_init(dir_pin_);
    gpio_set_dir(dir_pin_, GPIO_OUT);
    gpio_put(dir_pin_, false);

    // Find out (and save) which hardware (PWM slice & channel) are connected
    // to this GPIO.
    slice_num_ = pwm_gpio_to_slice_num(torque_pwm_pin_);
    gpio_channel_ = pwm_gpio_to_channel(torque_pwm_pin_);


    // Set period of 100 cycles (0 to 99 inclusive) (reg TOP value).
    pwm_set_wrap(slice_num_, 99);
    // Clear output duty cycle on startup.
    set_duty_cycle(0);
    set_pwm_frequency(DEFAULT_PWM_FREQ_HZ);


    // Enabling / Disabling PWM must be done by changing the duty cycle
    // and leaving the slice enabled bc disabling the slice leaves the GPIO
    // fixed in its current state.
    pwm_set_enabled(slice_num_, true);
}


BrushedMotorController::~BrushedMotorController()
{
    disable_output();
    pwm_set_enabled(slice_num_, false);
    // Set GPIOs to inputs.
    gpio_init_mask((1 << torque_pwm_pin_) | (1 << dir_pin_));
}


void BrushedMotorController::set_duty_cycle(uint8_t duty_cycle_percentage)
{
    // Clamp output.
    if (duty_cycle_percentage > 100)
    {
        duty_cycle_percentage = 100;
    }
    // Clear output duty cycle on startup.
    pwm_set_chan_level(slice_num_, gpio_channel_, duty_cycle_percentage);

    // Save it for enabling/disabling.
    duty_cycle_ = duty_cycle_percentage;
}


void BrushedMotorController::set_pwm_frequency(uint32_t freq_hz)
{
    // Configure for n[Hz] period broken down into PWM_STEP_INCREMEMENTS.
    // requested value must be within [0.0, 256.0]
    float new_freq_div = SYSTEM_CLOCK / freq_hz * PWM_STEP_INCREMENTS;

    // Default to 20[KHz].
    if (freq_hz < DIVIDER_MIN_FREQ_HZ || freq_hz > DRIVER_MAX_FREQ_HZ)
    {
        // Default is 62.5.
        new_freq_div = SYSTEM_CLOCK / DEFAULT_PWM_FREQ_HZ * PWM_STEP_INCREMENTS;
    }
    pwm_set_clkdiv(slice_num_, new_freq_div);
}


void BrushedMotorController::move_ms(uint32_t milliseconds)
{
    // increase the movement time.
    set_move_time_ms_ = milliseconds;
}


void BrushedMotorController::move_relative_angle(float angle)
{
    // increase the movement angle.
}


void BrushedMotorController::update()
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
            if (set_move_time_ms_ > 0 &&
                duty_cycle_ > 0) // Only move if a speed was set.
            {
                next_state = TIME_MOVE;
            }
            else if (set_move_angle_ticks_ != 0 &&
                     duty_cycle_ > 0) // Only move if a speed was set.
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
        enable_output();
    }
    else if (state_ == TIME_MOVE && next_state == IDLE)
    {
        move_start_time_ms_ = 0;
        disable_output();
    }
    else {}

    // Change states.
    state_ = next_state;
}
