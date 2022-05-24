#ifndef CPU_ENCODER_H
#define CPU_ENCODER_H

#include <stdint.h>

class CPUEncoder
{
public:
    /**
     * \brief Constructor. Wrapper for an encoder who's value is being written
     *        to a specified location in memory.
     * \details This class provides an interface to reading encoder ticks where
     *          the encoder ticks are currently being written to a specified
     *          location in memory. In this implementation, Core1 of the
     *          Pi Pico is dedicated to polling a number of encoders and
     *          writing their data into a double buffer. This class
     *          provides a simple, replacable, interface to accessing the
     *          encoder count.
     */
    CPUEncoder(uint32_t* base_address_ptr, uint32_t offset);

    ~CPUEncoder();

    /**
     * \brief return the current encoder ticks
     */
    int32_t get_ticks();

    /**
     * \brief reset the encoder counter.
     * \note offsetting is handled in this class since 2's complement math
     *       handles rollover gracefully and handling it at the buffer would
     *       require mutexes.
     */
    void zero();

private:
    uint32_t* base_address_ptr_;
    uint32_t offset_;

    int32_t tick_offset_; // For "zeroing" the encoder count.
};

#endif // CPU_ENCODER_H
