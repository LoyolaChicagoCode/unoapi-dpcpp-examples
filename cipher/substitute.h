#ifndef SUBSTITUTE_H_
#define SUBSTITUTE_H_

#include <sycl/sycl.hpp>

template <class Container>
SYCL_EXTERNAL void substitute(
    Container & byte_map,
    Container & input_message,
    Container & output_message,
    const size_t & input_message_size,
    const uint & grain_size,
    const size_t & i
)
{
    const auto start = i * grain_size;
    auto end = (i + 1) * grain_size;
    if (end > input_message_size) { end = input_message_size; }
    for (int j = start; j < end; j++) {
        output_message[j] = byte_map[std::to_integer<uint8_t>(input_message[j])];
    }
}

#endif