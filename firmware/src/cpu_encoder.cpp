#include <cpu_encoder.h>

CPUEncoder::CPUEncoder(uint32_t* base_address_ptr,
                       uint32_t offset)
    :base_address_ptr_{base_address_ptr}, offset_{offset}
{
}


CPUEncoder::~CPUEncoder(){}


int32_t CPUEncoder::get_ticks()
{
    // do pointer arithmatic; then retrieve the value at the address location.
    return *(base_address_ptr_ + offset_);
}


void CPUEncoder::zero()
{
    // Not implemented. This needs to be implemented with a mutex since both
    // cores will be changing a shared value.
    //reset_ptr |= (1 << offset_);
}
