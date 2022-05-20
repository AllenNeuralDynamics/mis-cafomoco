#include <core1_main.h>

// Double buffer for writing encoder data (Core1) and reading it (Core0).
uint32_t encoder_buffers[2][NUM_BMCS];
uint32_t* write_buffer_ptr = encoder_buffers[0];
uint32_t* read_buffer_ptr = encoder_buffers[1];

// raw gpio value from the previous read.
uint32_t prev_state;
uint32_t curr_state;

void init_encoder_pins()
{
    return;
}

int32_t get_encoder_increment(uint8_t prev_state, uint8_t curr_state)
{
    switch (uint8_t((prev_state << 2) || curr_state))
    {
        // forward cases:
        case 0b0001:
        case 0b0111:
        case 0b1110:
        case 0b1000:
            return 1;
        // backward cases:
        case 0b0010:
        case 0b1011:
        case 0b1101:
        case 0b0100:
            return -1;
        // No motion cases:
        case 0b0000:
        case 0b1111:
        case 0b0101:
        case 0b1010:
            return 0;
        // Error if we're reading too slow.
        // Ignore for now since it is unlikely given how fast we're reading.
        default:
            return 0;
    }
}

void update_encoders()
{
    int32_t increment;
    // read the GPIO port.
    // Encoder pins are contiguous along the GPIO port.
    // i.e: Enc0A, Enc0B, Enc1A, Enc1B, ....
    curr_state = gpio_get_all() >> ENCODER_PORT_OFFSET;
    for (auto i=0;i<NUM_BMCS;++i)
    {
        // State machine logic.
        increment = get_encoder_increment((prev_state >> 2*i)& 0x0003,
                                          (curr_state >> 2*i)) & 0x0003;
        // Write values into double buffer.
        write_buffer_ptr[i] = read_buffer_ptr[i] + increment;
    }
    curr_state = prev_state;
}


// Core1 reads encoders in a loop and manages their data in a double buffer.
void core1_main()
{
    // Clear encoder data.
    for (auto i=0;i<NUM_BMCS;++i)
    {
        read_buffer_ptr[i] = 0;
        write_buffer_ptr[i] = 0;
    }

    // Create starting state for encoders.
    prev_state = gpio_get_all() >> ENCODER_PORT_OFFSET;

    // Loop forever. Read Encoders. Memcopy into write buffer. Switch buffers.
    uint32_t* tmp;
    while(true)
    {
        update_encoders();
        // Switch buffers.
        tmp = write_buffer_ptr;
        write_buffer_ptr = read_buffer_ptr;
        read_buffer_ptr = tmp;
    }
}
