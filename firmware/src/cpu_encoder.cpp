#include <cpu_encoder.h>

CPUEncoder::CPUEncoder(int32_t* base_address_ptr,
                       uint32_t offset)
    :base_address_ptr_{base_address_ptr}, offset_{offset}, tick_offset_{0}
{
}


CPUEncoder::~CPUEncoder(){}


int32_t CPUEncoder::get_ticks()
{
    // do pointer arithmatic; then retrieve the value at the address location.
    return *(base_address_ptr_ + offset_) - tick_offset_;
}


void CPUEncoder::zero()
{
    tick_offset_ = 0;
    tick_offset_ = get_ticks();
}
