#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H
#include <stdint.h>
#include <pico/stdlib.h>
#include <hardware/pwm.h>
#include <cpu_encoder.h>
#include <en_dir_motor_driver.h>
#ifdef DEBUG
#include <stdio.h>
#endif

// All BMC States that do not return immediately.


class MotorController
{
public:
    enum dir_t
    {
        FORWARD,
        REVERSE
    };

    enum BMCState
    {
        IDLE,
        TIME_MOVE,
        DIST_MOVE,
        HOMING,
        ERROR // if we're stuck moving.
    };

    /**
     * \brief constructor. Connect to PWM hardware, set dir_pin to logic 0;
     *        set torque_pwm_pin to 0.
     * \param motor_driver the motor driver.
     * \param encoder the position feedback.
     */
    // TODO: motor_driver interface should be abstract.
    MotorController(EnDirMotorDriver& motor_driver,
                    CPUEncoder& encoder);


    /**
     * \brief destructor. Leave the outputs off and configured as input.
     */
    ~MotorController();


    /**
     *\brief sets the duty cycle to a value between 0 (off) and 100 (fully on).
     */
    void set_speed_percentage(uint8_t speed_percentage);


    /**
     *\brief sets the pwm frequency in Hz. Between 5[KHz] and 500[KHz]
     */
    void set_pwm_frequency(uint32_t freq_hz);



    /**
     * \brief set the direction
     */
    void set_dir(dir_t direction);

    /**
     * \brief move for a set number of milliseconds
     */
    void move_ms(uint32_t milliseconds);

    /**
     * \brief
     */
    void move_relative_angle(float angle);

    /**
     * \brief called as quickly as possible to address pending tasks.
     */
    void update();

    /**
     * \brief called in an interrupt.
     * \note inline
     */
    void hardware_update()
    {
        int32_t curr_ticks = encoder_.get_ticks();
        int32_t measured_speed = (curr_ticks - prev_encoder_ticks_) *
                                 HARDWARE_UPDATE_RATE_HZ;
        // simple lowpass filter on measured speed.
        measured_speed_ = 15*(measured_speed_ / 16) + (measured_speed / 16);
        prev_encoder_ticks_ = curr_ticks;
    }


    BMCState state_;

private:
    EnDirMotorDriver& motor_driver_;
    CPUEncoder& encoder_;

    // internal variable for handling currently set speed.
    uint8_t set_speed_;

    // internal variables for handling time move commands.
    uint32_t set_move_time_ms_;
    uint32_t move_start_time_ms_;

    // internal variables for handling distance move commands.
    int32_t set_move_angle_ticks_; // desired relative angle in encoder ticks.
    int32_t start_angle_ticks_;

    // These are modified within an interrupt.
    volatile bool stuck_moving_;
    volatile int32_t measured_speed_;
    volatile int32_t prev_encoder_ticks_;

    // Constants
    static const uint SYSTEM_CLOCK = 125000000;
    static const uint HARDWARE_UPDATE_RATE_HZ = 1000;

    static const uint PWM_STEP_INCREMENTS = 100;
    static const uint DEFAULT_PWM_FREQ_HZ = 20000; // Just beyond human hearing.

    // PWM Frequency range bounds.
    static const uint DIVIDER_MIN_FREQ_HZ = 5000;
    static const uint DRIVER_MAX_FREQ_HZ = 500000;

};

#endif // MOTOR_CONTROLLER_H
