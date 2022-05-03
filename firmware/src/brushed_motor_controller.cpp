#include <brushed_motor_controller.h>
// Assumes a 125MHz system clock.

BrushedMotorController::BrushedMotorController(uint8_t torque_pwm_pin, uint8_t dir_pin)
    :torque_pwm_pin_{torque_pwm_pin}, dir_pin_{dir_pin}
{
// inpired by:
// https://github.com/raspberrypi/pico-examples/blob/master/pwm/hello_pwm/hello_pwm.c#L14-L29
    // Allocate torque_pwm_pin for pwm; allocate dir pin as output.
    gpio_set_function(torque_pwm_pin_, GPIO_FUNC_PWM);

    gpio_init(dir_pin_);
    gpio_set_dir(dir_pin_, GPIO_OUT);
    gpio_put(dir_pin_, false);

    // Find out (and save) which hardware (PWM slice & channel) are connected to this GPIO.
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


/**
 * \brief set the duty cycle to a value between 0 and 100
 */
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
